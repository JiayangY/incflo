#ifndef _DIVOP_CONV_HPP_
#define _DIVOP_CONV_HPP_

//#include <mfix_F.H>
#include <incflo.H>
#include <AMReX_REAL.H>
#include <AMReX_BLFort.H>
#include <AMReX_SPACE.H>

void 
compute_divop_conv(
              Box& bx,
              MultiFab& divergence,
              MFIter* mfi,
              FArrayBox& fx,
              FArrayBox& fy,
              FArrayBox& fz,
              Array<const MultiCutFab*, AMREX_SPACEDIM>& areafrac,
              Array<const MultiCutFab*, AMREX_SPACEDIM>& facecent,
              const EBCellFlagFab& flags_fab,
              const MultiFab* volfrac,
              const MultiCutFab* bndrycent_fab,
              Box& domain,
              const int cyclic_x,
              const int cyclic_y,
              const int cyclic_z,
              const Real* dx);
#endif
