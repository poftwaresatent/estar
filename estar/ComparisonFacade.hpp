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


#ifndef ESTAR_COMPARISON_FACADE_HPP
#define ESTAR_COMPARISON_FACADE_HPP


#include <estar/FacadeWriteInterface.hpp>
#include <boost/shared_ptr.hpp>


namespace estar {
  
  
  class FacadeReadInterface;
  class Facade;
  
  
  class ComparisonFacade
    : public FacadeWriteInterface
  {
  private:
    ComparisonFacade(boost::shared_ptr<Facade> master,
		     bool auto_reset_master,
		     bool auto_flush_master,
		     boost::shared_ptr<Facade> sample,
		     bool auto_reset_sample,
		     bool auto_flush_sample);
    
  public:
    /**
       Uses Facade::Create() to initialize two identical Facades, one
       will be used as master, the other as sample.
    */
    static boost::shared_ptr<ComparisonFacade>
    Create(const std::string & kernel_name,
	   size_t xsize,
	   size_t ysize,
	   double scale,
	   int connect_diagonal,
	   FILE * dbgstream);
    
    /**
       Uses Facade::Create() to initialize two possibly different
       Facades.
    */
    static boost::shared_ptr<ComparisonFacade>
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
	   FILE * dbgstream);
    
    /**
       Uses Facade::CreateDefault() to initialize two identical
       Facades, one will be used as master, the other as sample.
    */
    static boost::shared_ptr<ComparisonFacade>
    CreateDefault(size_t xsize, size_t ysize, double scale);
    
    /**
       \return Read access to the "master" (or ground-truth) Facade,
       which is the one that the "sample" is compared against.
    */
    boost::shared_ptr<FacadeReadInterface const> GetMaster() const;
    
    /**
       \return Read access to the "sample" (or trial) Facade, which is
       the one that is compared against the "master".
    */
    boost::shared_ptr<FacadeReadInterface const> GetSample() const;
    
    /**
       Implements FacadeWriteInterface::SetMeta() by setting the meta
       on both master and sample. Flags the master for reset if the
       meta is actually changed.
    */
    virtual void SetMeta(size_t ix, size_t iy, double meta);
    
    /**
       Implements FacadeWriteInterface::InitMeta() by initializing the
       meta on both master and sample.
    */
    virtual void InitMeta(size_t ix, size_t iy, double meta);
    
    /**
       Implements FacadeWriteInterface::AddGoal() by adding the goal
       to both master and sample, and flagging the master for reset
       (unless the cell was already a goal with the same value).
    */
    virtual void AddGoal(size_t ix, size_t iy, double value);
    
    /**
       Implements FacadeWriteInterface::RemoveAllGoals() by removing
       all goals from both master and sample. Flags the master for
       reset.
    */
    virtual void RemoveAllGoals();
    
    /**
       Implements FacadeWriteInterface::ComputeOne(). Forwards the
       call as-is to the sample. But for the master it checks for
       pending reset requests and then repeatedly calls its
       ComputeOne().
    */
    virtual void ComputeOne();
     
    /**
       Implements FacadeWriteInterface::Reset() by resetting both
       master and sample.
    */
    virtual void Reset();
    
    
  private:
    boost::shared_ptr<Facade> m_master;
    bool m_auto_reset_master;
    bool m_auto_flush_master;
    bool m_reset_master;
    boost::shared_ptr<Facade> m_sample;
    bool m_auto_reset_sample;
    bool m_auto_flush_sample;
    bool m_reset_sample;
  };
  
} // namespace estar

#endif // ESTAR_COMPARISON_FACADE_HPP
