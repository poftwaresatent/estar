/* 
 * Copyright (C) 2007 Roland Philippsen <roland dot philippsen at gmx net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#include "ComparisonFacade.hpp"
#include "Facade.hpp"
#include "FacadeReadInterface.hpp"
#include "Algorithm.hpp"
#include "Kernel.hpp"
#include "pdebug.hpp"


namespace estar {
  
  
  ComparisonFacade::
  ComparisonFacade(boost::shared_ptr<Facade> master,
		   boost::shared_ptr<Facade> sample)
    : m_master(master),
      m_sample(sample)
  {
  }
  
  
  boost::shared_ptr<ComparisonFacade> ComparisonFacade::
  Create(const std::string & master_kernel_name,
	 double master_scale,
	 GridOptions const & master_grid_options,
	 AlgorithmOptions const & master_algo_options,
	 const std::string & sample_kernel_name,
	 double sample_scale,
	 GridOptions const & sample_grid_options,
	 AlgorithmOptions const & sample_algo_options,
	 FILE * dbgstream)
  {
    boost::shared_ptr<ComparisonFacade> result;
    boost::shared_ptr<Facade>
      master(Facade::Create(master_kernel_name,
			    master_scale,
			    master_grid_options,
			    master_algo_options,
			    dbgstream));
    if ( ! master) {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s(): Facade::Create() failed for master\n",
		__FUNCTION__);
      return result;
    }
    boost::shared_ptr<Facade>
      sample(Facade::Create(sample_kernel_name,
			    sample_scale,
			    sample_grid_options,
			    sample_algo_options,
			    dbgstream));
    if ( ! sample) {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s(): Facade::Create() failed for sample\n",
		__FUNCTION__);
      return result;
    }
    result.reset(new ComparisonFacade(master, sample));
    return result;
  }
  
  
  boost::shared_ptr<ComparisonFacade> ComparisonFacade::
  CreateDefault(ssize_t xsize, ssize_t ysize, double scale)
  {
    GridOptions grid_options(0, xsize, 0, ysize);
    AlgorithmOptions master_algo_options;
    master_algo_options.auto_reset = true;
    master_algo_options.auto_flush = true;
    return Create("lsm",
		  scale,
		  grid_options,
		  master_algo_options,
		  "lsm",
		  scale,
		  grid_options,
		  AlgorithmOptions(),
		  0);
  }
  
  
  boost::shared_ptr<FacadeReadInterface const> ComparisonFacade::
  GetMaster() const
  {
    return m_master;
  }
  
  
  boost::shared_ptr<FacadeReadInterface const> ComparisonFacade::
  GetSample() const
  {
    return m_sample;
  }
  
  
  bool ComparisonFacade::
  SetMeta(ssize_t ix, ssize_t iy, double meta)
  {
    bool const ok(m_master->SetMeta(ix, iy, meta));
    return m_sample->SetMeta(ix, iy, meta) && ok;
  }
  
  
  bool ComparisonFacade::
  AddGoal(ssize_t ix, ssize_t iy, double value)
  {
    bool const ok(m_master->AddGoal(ix, iy, value));
    return m_sample->AddGoal(ix, iy, value) && ok;
  }
  
  
  void ComparisonFacade::
  RemoveAllGoals()
  {
    m_master->RemoveAllGoals();
    m_sample->RemoveAllGoals();
  }
  
  
  void ComparisonFacade::
  ComputeOne()
  {
    m_master->ComputeOne();
    m_sample->ComputeOne();
  }
  
  
  void ComparisonFacade::
  Reset()
  {
    m_master->Reset();
    m_sample->Reset();
  }
  
  
  size_t ComparisonFacade::
  AddRange(ssize_t xbegin, ssize_t xend,
	   ssize_t ybegin, ssize_t yend,
	   double meta)
  {
    m_master->AddRange(xbegin, xend, ybegin, yend, meta);
    return m_sample->AddRange(xbegin, xend, ybegin, yend, meta);
  }
  
  
  bool ComparisonFacade::
  AddNode(ssize_t ix, ssize_t iy, double meta)
  {
    m_master->AddNode(ix, iy, meta);
    return m_sample->AddNode(ix, iy, meta);
  }
  
} // namespace estar
