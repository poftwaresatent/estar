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
#include "Grid.hpp"
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
	 FacadeOptions const & master_options,
	 const std::string & sample_kernel_name,
	 FacadeOptions const & sample_options,
	 size_t xsize,
	 size_t ysize,
	 double scale,
	 FILE * dbgstream)
  {
    boost::shared_ptr<ComparisonFacade> result;
    boost::shared_ptr<Facade>
      master(Facade::Create(master_kernel_name, xsize, ysize, scale,
			    master_options, dbgstream));
    if ( ! master) {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s(): Facade::Create() failed for master\n",
		__FUNCTION__);
      return result;
    }
    boost::shared_ptr<Facade>
      sample(Facade::Create(sample_kernel_name, xsize, ysize, scale,
			    sample_options, dbgstream));
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
  CreateDefault(size_t xsize, size_t ysize, double scale)
  {
    FacadeOptions master_options;
    master_options.auto_reset = true;
    master_options.auto_flush = true;
    return Create("lsm", master_options, "lsm", FacadeOptions(),
		  xsize, ysize, scale, 0);
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
  
  
  void ComparisonFacade::
  SetMeta(size_t ix, size_t iy, double meta)
  {
    m_master->SetMeta(ix, iy, meta);
    m_sample->SetMeta(ix, iy, meta);
  }
  
  
  void ComparisonFacade::
  InitMeta(size_t ix, size_t iy, double meta)
  {
    m_master->InitMeta(ix, iy, meta);
    m_sample->InitMeta(ix, iy, meta);
  }
  
  
  void ComparisonFacade::
  AddGoal(size_t ix, size_t iy, double value)
  {
    m_master->AddGoal(ix, iy, value);
    m_sample->AddGoal(ix, iy, value);
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
  
} // namespace estar
