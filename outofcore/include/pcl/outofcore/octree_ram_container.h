#pragma once

/*
  Copyright (c) 2011, Urban Robotics Inc
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
  This code defines the octree used for point storage at Urban Robotics. Please
  contact Jacob Schloss <jacob.schloss@urbanrobotics.net> with any questions.
  http://www.urbanrobotics.net/
*/

// C++
#include <vector>

// Boost
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/random/mersenne_twister.hpp>

//todo - Consider using per-node RNG (it is currently a shared static rng,
//       which is mutexed. I did i this way to be sure that node of the nodes
//       had RNGs seeded to the same value). the mutex could effect performance

/** According to the Urban Robotics documentation, this class was not
 *  maintained with the rest of the disk_container code base. Watch
 *  out for drift.
 * \todo Action item for URCS - test this and determine the status of its functionality
 */

template<typename PointType>
class octree_ram_container
{
  public:
    
    octree_ram_container (const boost::filesystem::path& dir) { }
    
    inline void
    insertRange (const PointType* start, const boost::uint64_t count);

    inline void
    insertRange (const PointType* const * start, const boost::uint64_t count);

    void
    readRange (const boost::uint64_t start, const boost::uint64_t count, std::vector<PointType>& v);

    /** \brief grab percent*count random points. points are NOT
     *   guaranteed to be unique (could have multiple identical points!)
     */
    void
    readRangeSubSample (const boost::uint64_t start, const boost::uint64_t count, const double percent,
                        std::vector<PointType>& v);

    inline boost::uint64_t
    size () const
    {
      return container.size ();
    }

    inline void
    flush (const bool forceCacheDeAlloc) { } //???

    inline void
    clear ()
    {
      container.clear ();
    }

    void
    convertToXYZ (const boost::filesystem::path& path);

  private:
    //no copy construction
    octree_ram_container (const octree_ram_container& rval) { }

    octree_ram_container&
    operator= (const octree_ram_container& rval) { }

    //the actual container
    //std::deque<PointType> container;
    std::vector<PointType> container;

    static boost::mutex rng_mutex;
    static boost::mt19937 rand_gen;
};


