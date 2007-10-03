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
      m_sample(sample),
      m_reset_master(false)
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
    result.reset(new ComparisonFacade(master, sample));
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
    result.reset(new ComparisonFacade(master, sample));
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
    m_reset_master = true;
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
    if (m_master->IsGoal(ix, iy)
	&& (absval(m_master->GetValue(ix, iy) - value) < epsilon))
      return;
    m_reset_master = true;
    m_master->AddGoal(ix, iy, value);
    m_sample->AddGoal(ix, iy, value);
  }
  
  
  void ComparisonFacade::
  RemoveAllGoals()
  {
    m_reset_master = true;
    m_master->RemoveAllGoals();
    m_sample->RemoveAllGoals();
  }
  
  
  void ComparisonFacade::
  ComputeOne()
  {
    if (m_reset_master) {
      m_reset_master = false;
      PVDEBUG("reset master\n");
      m_master->Reset();
    }
    if (m_master->HaveWork())
      PVDEBUG("propagate master\n");
    while (m_master->HaveWork())
      m_master->ComputeOne();
    if (m_sample->HaveWork())
      PVDEBUG("propagate sample\n");
    m_sample->ComputeOne();
  }
  
  
  void ComparisonFacade::
  Reset()
  {
    m_master->Reset();
    m_sample->Reset();
  }
  
} // namespace estar
