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
		   bool auto_reset_master,
		   bool auto_flush_master,
		   boost::shared_ptr<Facade> sample,
		   bool auto_reset_sample,
		   bool auto_flush_sample)
    : m_master(master),
      m_auto_reset_master(auto_reset_master),
      m_auto_flush_master(auto_flush_master),
      m_reset_master(false),
      m_sample(sample),
      m_auto_reset_sample(auto_reset_sample),
      m_auto_flush_sample(auto_flush_sample),
      m_reset_sample(false)
  {
  }
  
  
  boost::shared_ptr<ComparisonFacade> ComparisonFacade::
  Create(const std::string & kernel_name,
	 size_t xsize,
	 size_t ysize,
	 double scale,
	 int connect_diagonal,
	 FILE * dbgstream)
  {
    boost::shared_ptr<ComparisonFacade> result;
    boost::shared_ptr<Facade>
      master(Facade::Create(kernel_name, xsize, ysize, scale,
			    connect_diagonal, dbgstream));
    if ( ! master) {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s(): Facade::Create() failed for master\n",
		__FUNCTION__);
      return result;
    }
    boost::shared_ptr<Facade>
      sample(Facade::Create(kernel_name, xsize, ysize, scale,
			    connect_diagonal, dbgstream));
    if ( ! sample) {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s(): Facade::Create() failed for sample\n",
		__FUNCTION__);
      return result;
    }
    result.reset(new ComparisonFacade(master, true, true,
				      sample, false, false));
    return result;
  }
  
  
  boost::shared_ptr<ComparisonFacade> ComparisonFacade::
  Create(const std::string & master_kernel_name,
	 int master_connect_diagonal,
	 bool master_auto_reset,
	 bool master_auto_flush,
	 const std::string & sample_kernel_name,
	 int sample_connect_diagonal,
	 bool sample_auto_reset,
	 bool sample_auto_flush,
	 size_t xsize,
	 size_t ysize,
	 double scale,
	 FILE * dbgstream)
  {
    boost::shared_ptr<ComparisonFacade> result;
    boost::shared_ptr<Facade>
      master(Facade::Create(master_kernel_name, xsize, ysize, scale,
			    master_connect_diagonal, dbgstream));
    if ( ! master) {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s(): Facade::Create() failed for master\n",
		__FUNCTION__);
      return result;
    }
    boost::shared_ptr<Facade>
      sample(Facade::Create(sample_kernel_name, xsize, ysize, scale,
			    sample_connect_diagonal, dbgstream));
    if ( ! sample) {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s(): Facade::Create() failed for sample\n",
		__FUNCTION__);
      return result;
    }
    result.reset(new ComparisonFacade(master,
				      master_auto_reset,
				      master_auto_flush,
				      sample,
				      sample_auto_reset,
				      sample_auto_flush));
    return result;
  }


  
  
  boost::shared_ptr<ComparisonFacade> ComparisonFacade::
  CreateDefault(size_t xsize, size_t ysize, double scale)
  {
    boost::shared_ptr<ComparisonFacade> result;
    boost::shared_ptr<Facade>
      master(Facade::CreateDefault(xsize, ysize, scale));
    if ( ! master)
      return result;
    boost::shared_ptr<Facade>
      sample(Facade::CreateDefault(xsize, ysize, scale));
    if ( ! sample)
      return result;
    result.reset(new ComparisonFacade(master, true, true,
				      sample, false, false));
    return result;
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
    if (absval(m_master->GetMeta(ix, iy) - meta) < epsilon)
      return;

    m_master->SetMeta(ix, iy, meta);
    if (m_auto_reset_master)
      m_reset_master = true;

    m_sample->SetMeta(ix, iy, meta);
    if (m_auto_reset_sample)
      m_reset_sample = true;
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
    if (m_master->IsGoal(ix, iy)
	&& (absval(m_master->GetValue(ix, iy) - value) < epsilon))
      return;
    
    m_master->AddGoal(ix, iy, value);
    if (m_auto_reset_master)
      m_reset_master = true;
    
    m_sample->AddGoal(ix, iy, value);
    if (m_auto_reset_sample)
      m_reset_sample = true;
  }
  
  
  void ComparisonFacade::
  RemoveAllGoals()
  {
    m_master->RemoveAllGoals();
    if (m_auto_reset_master)
      m_reset_master = true;

    m_sample->RemoveAllGoals();
    if (m_auto_reset_sample)
      m_reset_sample = true;
  }
  
  
  void ComparisonFacade::
  ComputeOne()
  {
    if (m_reset_master) {
      m_reset_master = false;
      PDEBUG("reset master\n");
      m_master->Reset();
    }
    
    if (m_auto_flush_master && m_master->HaveWork()) {
      PDEBUG("flush master\n");
      while (m_master->HaveWork())
	m_master->ComputeOne();
    }
    else
      m_master->ComputeOne();

    if (m_reset_sample) {
      m_reset_sample = false;
      PDEBUG("reset sample\n");
      m_sample->Reset();
    }
    
    if (m_auto_flush_sample && m_sample->HaveWork()) {
      PDEBUG("flush sample\n");
      while (m_sample->HaveWork())
	m_sample->ComputeOne();
    }
    else
      m_sample->ComputeOne();
  }
  
  
  void ComparisonFacade::
  Reset()
  {
    m_master->Reset();
    m_sample->Reset();
  }
  
} // namespace estar
