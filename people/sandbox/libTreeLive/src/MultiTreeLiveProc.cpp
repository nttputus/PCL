/**
 * @authors: Cedric Cagniart, Koen Buys
 */
#include "commonTrees/TRun.h"
#include "commonTrees/HandleError.h"

#include <libTreeLive/TreeLive.h>
#include "CUDATree.h"

#include <cuda_runtime_api.h>
#include "kernels/CUDA_runMultiTree.h"

#include <iostream>

using Tree::Node;
using Tree::Label;
using Tree::loadTree;
using Tree::focal;


namespace TreeLive
{
MultiTreeLiveProc::MultiTreeLiveProc(std::istream& is)
{
	// load the tree file
	std::vector<Node>  nodes;
	std::vector<Label> leaves;
	
	std::cout<<"(I) : MultiTreeLiveProc(): before loadTree()" << std::endl;

	// this might throw but we haven't done any malloc yet
	int height = loadTree(is, nodes, leaves );  

	std::cout<<"(I) : MultiTreeLiveProc(): before CUDATree()" << std::endl;
	CUDATree* tree = new CUDATree(height, nodes, leaves);

	std::cout<<"(I) : MultiTreeLiveProc(): before push_back()" << std::endl;
	m_trees.push_back(tree);

        std::cout<<"(I) : MultiTreeLiveProc(): CUDA Tree created, going to cudaMalloc now " << std::endl;

	// alloc cuda
	HANDLE_ERROR( cudaMalloc(&m_dmap_device,      640*480*sizeof(uint16_t)));
	HANDLE_ERROR( cudaMalloc(&m_multilmap_device, 640*480*4*sizeof(uint8_t)));
	HANDLE_ERROR( cudaMalloc(&m_lmap_device,      640*480*sizeof(uint8_t)));

        std::cout<<"(I) : MultiTreeLiveProc(): CUDA Tree cudaMalloc done" << std::endl;

}


MultiTreeLiveProc::~MultiTreeLiveProc() 
{
	for( std::vector<CUDATree*>::iterator t_itr = m_trees.begin(); 
	                                         t_itr != m_trees.end(); t_itr ++ ){
		delete *t_itr;
	}

	cudaFree(m_dmap_device);
	cudaFree(m_multilmap_device);
	cudaFree(m_lmap_device);
}


#define TLIVEEXCEPT(msg) \
	throw std::runtime_error(msg);

void MultiTreeLiveProc::addTree(std::istream& is)
{
	if( m_trees.size() >= 4 ) TLIVEEXCEPT("there are already 4 trees in this Processor")

	std::vector<Node>  nodes;
	std::vector<Label> leaves;
	// this might throw but we haven't done any malloc yet
	int height = loadTree(is, nodes, leaves );  
	CUDATree* tree = new CUDATree(height, nodes, leaves);
	m_trees.push_back(tree);
}



void MultiTreeLiveProc::process(const cv::Mat& dmap,
                                cv::Mat&       lmap )
{
	process( dmap, lmap, std::numeric_limits<Tree::Attrib>::max() );
}


void MultiTreeLiveProc::process(const cv::Mat& dmap,
                                cv::Mat&       lmap,
                                int        FGThresh)
{
	if( dmap.depth()       != CV_16U ) TLIVEEXCEPT("depth has incorrect channel type")
	if( dmap.channels()    != 1 )      TLIVEEXCEPT("depth has incorrect channel count")
	if( dmap.size().width  != 640 )    TLIVEEXCEPT("depth has incorrect width")
	if( dmap.size().height != 480 )    TLIVEEXCEPT("depth has incorrect height")
	if( !dmap.isContinuous() )         TLIVEEXCEPT("depth has non contiguous rows")

	// alloc the buffer if it isn't done yet
	lmap.create( 480, 640, CV_8UC(1) );
	
	// copy depth to cuda
	cudaMemcpy(m_dmap_device, (const void*) dmap.data, 
	                          640*480*sizeof(uint16_t), cudaMemcpyHostToDevice);

	// 1 - run the multi passes
	int numTrees = m_trees.size();
	for(int ti=0; ti<numTrees; ++ti ) {
		CUDATree* t = m_trees[ti];
		if( FGThresh == std::numeric_limits<Tree::Attrib>::max() ) {
			CUDA_runMultiTreePass( ti, 640,480, focal,  
	                                             t->treeHeight(), t->numNodes(),
	                                      t->nodes_device(), t->leaves_device(),
	                                        m_dmap_device, m_multilmap_device );
		}
		else {
			CUDA_runMultiTreePassFG( ti, FGThresh, 640,480, focal,  
	                                             t->treeHeight(), t->numNodes(),
	                                      t->nodes_device(), t->leaves_device(),
	                                        m_dmap_device, m_multilmap_device );
		}
	}
	// 2 - run the merging 
	CUDA_runMultiTreeMerge(m_trees.size(), 640,480, 
	                            m_dmap_device, m_multilmap_device, m_lmap_device);
	// 3 - download back from cuda ( the copy will execute all the kernels at once)
	cudaMemcpy((Label*)(lmap.data), m_lmap_device,
	                             640*480*sizeof(Label), cudaMemcpyDeviceToHost);
}


#undef TLIVEEXCEPT


} // end namespace TreeLive



