#include <AMReX_ParmParse.H>
#include <AMReX_BC_TYPES.H>
#include <AMReX_Box.H>

#include <incflo.H>
#include <boundary_conditions_F.H>
#include <embedded_boundaries_F.H>
#include <setup_F.H>

void incflo::ReadParameters()
{
    {
        // Variables without prefix in inputs file
		ParmParse pp;

		pp.query("stop_time", stop_time);
		pp.query("max_step", max_step);
		pp.query("steady_state", steady_state);
    }
	{
        // Prefix amr
		ParmParse pp("amr");

		pp.query("regrid_int", regrid_int);
        pp.query("refine_cutcells", refine_cutcells);

		pp.query("check_file", check_file);
		pp.query("check_int", check_int);
		pp.query("restart", restart_file);

		pp.query("plot_file", plot_file);
		pp.query("plot_int", plot_int);
		pp.query("plot_per", plot_per);

        // Which variables to write to plotfile
        pltVarCount = 0;

        pp.query("plt_vel",        plt_vel  );
        pp.query("plt_gradp",      plt_gradp);
        pp.query("plt_rho",        plt_rho  );
        pp.query("plt_p",          plt_p    );
        pp.query("plt_eta",        plt_eta  );
        pp.query("plt_vort",       plt_vort );
        pp.query("plt_strainrate", plt_strainrate);
        pp.query("plt_stress"    , plt_stress);
        pp.query("plt_divu",       plt_divu );
        pp.query("plt_vfrac",      plt_vfrac);

        // Special test for CCSE regression test. Override all individual
        // flags and save all data to plot file.

        int plt_ccse_regtest = 0;
        pp.query("plt_ccse_regtest", plt_ccse_regtest);

        if(plt_ccse_regtest != 0) 
        {
            plt_vel        = 1;
            plt_gradp      = 1;
            plt_rho        = 1;
            plt_p          = 1;
            plt_eta        = 1;
            plt_vort       = 1;
            plt_strainrate = 1;
            plt_stress     = 1;
            plt_divu       = 1;
            plt_vfrac      = 1;
        }

        // Count the number of variables to save.
        if(plt_vel        == 1) pltVarCount += 3;
        if(plt_gradp      == 1) pltVarCount += 3;
        if(plt_rho        == 1) pltVarCount += 1;
        if(plt_p          == 1) pltVarCount += 1;
        if(plt_eta        == 1) pltVarCount += 1;
        if(plt_vort       == 1) pltVarCount += 1;
        if(plt_strainrate == 1) pltVarCount += 1;
        if(plt_stress     == 1) pltVarCount += 1;
        if(plt_divu       == 1) pltVarCount += 1;
        if(plt_vfrac      == 1) pltVarCount += 1;
	}
	{
        // Prefix incflo
		ParmParse pp("incflo");

        pp.query("verbose", incflo_verbose);
		pp.query("cfl", cfl);
		pp.query("fixed_dt", fixed_dt);
		pp.query("steady_state_tol", steady_state_tol);
        pp.query("initial_iterations", initial_iterations);
        pp.query("do_initial_proj", do_initial_proj);

        // Physics
		pp.queryarr("delp", delp, 0, 3);
		pp.queryarr("gravity", gravity, 0, 3);
        pp.query("ro_0", ro_0);
        AMREX_ALWAYS_ASSERT(ro_0 >= 0.0);

        // Initial conditions
        pp.query("probtype", probtype);
        pp.query("ic_u", ic_u);
        pp.query("ic_v", ic_v);
        pp.query("ic_w", ic_w);
        pp.query("ic_p", ic_p);

        // Fluid properties
        pp.query("mu", mu);
        AMREX_ALWAYS_ASSERT(mu > 0.0);

        // TODO: Make a rheology class
        fluid_model = "newtonian";
        pp.query("fluid_model", fluid_model);
        if(fluid_model == "newtonian")
        {
            amrex::Print() << "Newtonian fluid with"
                           << " mu = " << mu << std::endl;
        }
        else if(fluid_model == "powerlaw")
        {
            pp.query("n", n);
            AMREX_ALWAYS_ASSERT(n > 0.0);
            AMREX_ALWAYS_ASSERT_WITH_MESSAGE(n != 1.0,
                    "No point in using power-law rheology with n = 1");

            amrex::Print() << "Power-law fluid with"
                           << " mu = " << mu
                           << ", n = " << n <<  std::endl;
        }
        else if(fluid_model == "bingham")
        {
            pp.query("tau_0", tau_0);
            AMREX_ALWAYS_ASSERT_WITH_MESSAGE(tau_0 > 0.0,
                    "No point in using Bingham rheology with tau_0 = 0");

            pp.query("papa_reg", papa_reg);
            AMREX_ALWAYS_ASSERT_WITH_MESSAGE(papa_reg > 0.0,
                    "Papanastasiou regularisation parameter must be positive");

            amrex::Print() << "Bingham fluid with"
                           << " mu = " << mu
                           << ", tau_0 = " << tau_0
                           << ", papa_reg = " << papa_reg << std::endl;
        }
        else if(fluid_model == "hb")
        {
            pp.query("n", n);
            AMREX_ALWAYS_ASSERT(n > 0.0);
            AMREX_ALWAYS_ASSERT_WITH_MESSAGE(n != 1.0,
                    "No point in using Herschel-Bulkley rheology with n = 1");

            pp.query("tau_0", tau_0);
            AMREX_ALWAYS_ASSERT_WITH_MESSAGE(tau_0 > 0.0,
                    "No point in using Herschel-Bulkley rheology with tau_0 = 0");

            pp.query("papa_reg", papa_reg);
            AMREX_ALWAYS_ASSERT_WITH_MESSAGE(papa_reg > 0.0,
                    "Papanastasiou regularisation parameter must be positive");

            amrex::Print() << "Herschel-Bulkley fluid with"
                           << " mu = " << mu
                           << ", n = " << n
                           << ", tau_0 = " << tau_0
                           << ", papa_reg = " << papa_reg << std::endl;
        }
        else if(fluid_model == "smd")
        {
            pp.query("n", n);
            AMREX_ALWAYS_ASSERT(n > 0.0);

            pp.query("tau_0", tau_0);
            AMREX_ALWAYS_ASSERT_WITH_MESSAGE(tau_0 > 0.0,
                    "No point in using de Souza Mendes-Dutra rheology with tau_0 = 0");

            pp.query("eta_0", eta_0);
            AMREX_ALWAYS_ASSERT(eta_0 > 0.0);

            amrex::Print() << "de Souza Mendes-Dutra fluid with"
                           << " mu = " << mu
                           << ", n = " << n
                           << ", tau_0 = " << tau_0
                           << ", eta_0 = " << eta_0 << std::endl;
        }
        else
        {
            amrex::Abort("Unknown fluid_model! Choose either newtonian, powerlaw, bingham, hb, smd");
        }

        // Get cyclicity, (to pass to Fortran)
        Vector<int> is_cyclic(3);
        for(int dir = 0; dir < 3; dir++)
        {
            is_cyclic[dir] = geom[0].isPeriodic(dir);
        }

        // Loads constants given at runtime `inputs` file into the Fortran module "constant"
        fortran_get_data(is_cyclic.dataPtr(),
                         delp.dataPtr(), gravity.dataPtr(), &ro_0, &mu,
                         &ic_u, &ic_v, &ic_w, &ic_p,
                         &n, &tau_0, &papa_reg, &eta_0,
                         fluid_model.c_str(), fluid_model.size());
	}
    {
        // Prefix cylinder
		ParmParse pp("cylinder");
		pp.query("speed", cyl_speed);
    }
}

void incflo::PostInit(int restart_flag)
{
    // Set the BC types on domain boundary
    SetBCTypes();

    // Reset MAC projection object
    mac_projection.reset(new MacProjection(this, nghost, &ebfactory, probtype));
    mac_projection->set_bcs(bc_ilo, bc_ihi, bc_jlo, bc_jhi, bc_klo, bc_khi);

    poisson_equation.reset(new PoissonEquation(this, &ebfactory,
                                               bc_ilo, bc_ihi,
                                               bc_jlo, bc_jhi,
                                               bc_klo, bc_khi, nghost));

    diffusion_equation.reset(new DiffusionEquation(this, &ebfactory,
                                                   bc_ilo, bc_ihi,
                                                   bc_jlo, bc_jhi,
                                                   bc_klo, bc_khi, nghost, cyl_speed));

    // Initial fluid arrays: pressure, velocity, density, viscosity
    if(!restart_flag)
    {
        InitFluid();
    }

    // Set the background pressure and gradients in "DELP" cases
    SetBackgroundPressure();

    // Fill boundaries
    FillScalarBC();
    FillVelocityBC(cur_time, 0);

    // Project the initial velocity field to make it divergence free
    // Perform initial iterations to find pressure distribution
    if(!restart_flag)
    {
        if (do_initial_proj)
            InitialProjection();
        if (initial_iterations > 0)
            InitialIterations();
    }
}

void incflo::InitFluid()
{
	Real xlen = geom[0].ProbHi(0) - geom[0].ProbLo(0);
	Real ylen = geom[0].ProbHi(1) - geom[0].ProbLo(1);
	Real zlen = geom[0].ProbHi(2) - geom[0].ProbLo(2);

    for(int lev = 0; lev <= max_level; lev++)
    {
        Box domain(geom[lev].Domain());

        Real dx = geom[lev].CellSize(0);
        Real dy = geom[lev].CellSize(1);
        Real dz = geom[lev].CellSize(2);

        // We deliberately don't tile this loop since we will be looping
        //    over bc's on faces and it makes more sense to do this one grid at a time
        for(MFIter mfi(*ro[lev], false); mfi.isValid(); ++mfi)
        {
            const Box& bx = mfi.validbox();
            const Box& sbx = (*ro[lev])[mfi].box();
            init_fluid(sbx.loVect(), sbx.hiVect(),
                       bx.loVect(), bx.hiVect(),
                       domain.loVect(), domain.hiVect(),
                       (*ro[lev])[mfi].dataPtr(),
                       (*p[lev])[mfi].dataPtr(),
                       (*vel[lev])[mfi].dataPtr(),
                       (*eta[lev])[mfi].dataPtr(),
                       &dx, &dy, &dz,
                       &xlen, &ylen, &zlen, &probtype);
        }
    }
}

void incflo::SetBCTypes()
{
    for(int lev = 0; lev <= max_level; lev++)
    {
        Real dx = geom[lev].CellSize(0);
        Real dy = geom[lev].CellSize(1);
        Real dz = geom[lev].CellSize(2);
        Real xlen = geom[lev].ProbHi(0) - geom[lev].ProbLo(0);
        Real ylen = geom[lev].ProbHi(1) - geom[lev].ProbLo(1);
        Real zlen = geom[lev].ProbHi(2) - geom[lev].ProbLo(2);
        Box domain(geom[lev].Domain());

        set_bc_type(bc_ilo[lev]->dataPtr(), bc_ihi[lev]->dataPtr(),
                    bc_jlo[lev]->dataPtr(), bc_jhi[lev]->dataPtr(),
                    bc_klo[lev]->dataPtr(), bc_khi[lev]->dataPtr(),
                    domain.loVect(), domain.hiVect(),
                    &dx, &dy, &dz, &xlen, &ylen, &zlen, &nghost);
    }
}

void incflo::SetBackgroundPressure()
{
	Real xlen = geom[0].ProbHi(0) - geom[0].ProbLo(0);
	Real ylen = geom[0].ProbHi(1) - geom[0].ProbLo(1);
	Real zlen = geom[0].ProbHi(2) - geom[0].ProbLo(2);

	int delp_dir;
	set_delp_dir(&delp_dir);

    IntVect press_per = IntVect(geom[0].isPeriodic(0),
                                geom[0].isPeriodic(1),
                                geom[0].isPeriodic(2));

	// Here we set a separate periodicity flag for p0 because when we use
	// pressure drop (delp) boundary conditions we fill all variables *except* p0
	// periodically
    if(delp_dir > -1)
    {
        press_per[delp_dir] = 0;
    }
	p0_periodicity = Periodicity(press_per);

    for(int lev = 0; lev <= max_level; lev++)
    {
        Real dx = geom[lev].CellSize(0);
        Real dy = geom[lev].CellSize(1);
        Real dz = geom[lev].CellSize(2);

        Box domain(geom[lev].Domain());

        // We deliberately don't tile this loop since we will be looping
        //    over bc's on faces and it makes more sense to do this one grid at a time
        for(MFIter mfi(*ro[lev], false); mfi.isValid(); ++mfi)
        {
            const Box& bx = mfi.validbox();

            set_p0(bx.loVect(), bx.hiVect(),
                   domain.loVect(), domain.hiVect(),
                   BL_TO_FORTRAN_ANYD((*p0[lev])[mfi]),
                   &gp0[0],
                   &dx, &dy, &dz, &xlen, &ylen, &zlen,
                   &delp_dir,
                   bc_ilo[lev]->dataPtr(),
                   bc_ihi[lev]->dataPtr(),
                   bc_jlo[lev]->dataPtr(),
                   bc_jhi[lev]->dataPtr(),
                   bc_klo[lev]->dataPtr(),
                   bc_khi[lev]->dataPtr(),
                   &nghost);
        }
        p0[lev]->FillBoundary(p0_periodicity);
    }
}

//
// Perform initial pressure iterations
//
void incflo::InitialIterations()
{
    BL_PROFILE("incflo::InitialIterations()");

    int initialisation = 1;
	ComputeDt(initialisation);

    if(incflo_verbose)
    {
        amrex::Print() << "Doing initial pressure iterations with dt = " << dt << std::endl;
    }

    // Fill ghost cells
    FillScalarBC();
    FillVelocityBC(cur_time, 0);

    // Copy vel into vel_o
    for(int lev = 0; lev <= finest_level; lev++)
    {
        MultiFab::Copy(*vel_o[lev], *vel[lev], 0, 0, vel[lev]->nComp(), vel_o[lev]->nGrow());
    }

	for(int iter = 0; iter < initial_iterations; ++iter)
	{
        if(incflo_verbose) amrex::Print() << "\n In initial_iterations: iter = " << iter << "\n";

		ApplyPredictor();

        for(int lev = 0; lev <= finest_level; lev++)
        {
            // Replace vel by the original values
            MultiFab::Copy(*vel[lev], *vel_o[lev], 0, 0, vel[lev]->nComp(), vel[lev]->nGrow());
        }
        // Reset the boundary values (necessary if they are time-dependent)
        FillVelocityBC(cur_time, 0);
	}
}

// Project velocity field to make sure initial velocity is divergence-free
void incflo::InitialProjection()
{
    BL_PROFILE("incflo::InitialProjection()");

    if(incflo_verbose)
    {
        amrex::Print() << "Initial projection:" << std::endl;
    }

	// Need to add this call here so that the MACProjection internal arrays
	//  are allocated so that the cell-centered projection can use the MAC
	//  data structures and set_velocity_bcs routine
	mac_projection->update_internals();

	Real dummy_dt = 1.0;
	ApplyProjection(cur_time, dummy_dt);

    // Set nstep (initially -1) to 0, so that subsequent call to ApplyProjection()
    // use the correct decomposition.
    nstep = 0;


	// We set p and gp back to zero (p0 may still be still non-zero)
    for(int lev = 0; lev <= finest_level; lev++)
    {
        p[lev]->setVal(0.0);
        gp[lev]->setVal(0.0);
    }
}


