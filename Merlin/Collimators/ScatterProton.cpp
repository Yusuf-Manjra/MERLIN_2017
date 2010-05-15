#include <cmath>
#include <fstream>
#include "NumericalUtils/PhysicalUnits.h"
#include "NumericalUtils/utils.h"

#include "BeamModel/PSTypes.h"
#include "Collimators/ScatterProton.h"
#include "Random/RandomNG.h"

#include "AcceleratorModel/Apertures/TiltedAperture.hpp"

using namespace PhysicalUnits;

// calculate random small-angle Coulomb scattering
pair < double, double > CoulombScatterp (double x, double theta0)
{
	// x - material lenght in meters
	// theta0 - RMS scattering angle (plane)
	// See particle data book section 27.3
	static const double root12 = sqrt (12.0);

	double z1 = RandomNG::normal (0, 1);
	double z2 = RandomNG::normal (0, 1);

	double theta_plane = z2 * theta0;
	double y_plane = z1 * x * theta0 / root12 + x * theta_plane / 2;
	return make_pair (y_plane, theta_plane);
}

int ScatterProton (PSvector & p, double x, double E0, const Aperture * tap)
{
	//cout << "Scatter proton\t" << E0 << endl;
	//Check for possible badness
	if(tap == NULL)
	{
		std::cerr << "tap is NULL in ScatterProton.cpp - exiting." << std::endl;
		exit(EXIT_FAILURE);
	}

	if(tap->Material == NULL)
	{
		std::cerr << "Material is NULL in ScatterProton.cpp" << std::endl;
		exit(EXIT_FAILURE);
	}
	//std::cout << tap->sig_pN_tot << std::endl;
	double sig_pN_tot = tap->Material->sig_pN_tot;
	double sig_pN_in = tap->Material->sig_pN_in;
	double sig_R = tap->Material->sig_R;
	double dEdx = tap->Material->dEdx;
	double rho = tap->Material->rho;
	double A = tap->Material->A;
	double X0 = tap->Material->X0;

	if (X0 == 0)
	{
		cout << "X0 is zero, this is very bad" << endl;
		exit(EXIT_FAILURE);
	}

	/*
		double sig_pN_tot = 0;
		double sig_pN_in = tap->sig_pN_in;
		double sig_R = tap->sig_R;
		double dEdx = 0;
		double rho = tap->rho;
		double sig_pN_el = 0;
		double A = 0;
		double lambda_tot = 1E100;
	*/
	cout << "X0:\t" << X0 << endl;
	double lambda_tot = A * 1.e-6 / ((sig_pN_tot + sig_R) * 1.e-28 * rho * 6.023e23);	// total mean free path
	// double lambda_tot = A*1.e-6/((sig_pN_in)*1.e-28*rho*6.023e23); // total mean free path
	double delta_s = -lambda_tot * log (RandomNG::uniform (0, 1));
	//cout << "length=" << x << " lambda_tot="<<lambda_tot << " delta_s="<<delta_s<<endl;   
	cout << "lambda_tot:\t"<< lambda_tot << "\tdelta_s:\t" << delta_s << "\tx:\t" << x << endl;
	if (delta_s > x)
	{				//"coulomb" scatter
		static const double MAXDP = 1.0 - 1.0e-7;	// maximum allowed energy loss
		double E1 = E0 * (1 + p.dp ());	// particle energy
		// cout <<"E1= "<<E1<<endl;
		double t = x / X0;	// material length in radiation lengths
		/*    
		double t1 = t/log(2.0);
		double t2 = 0.5*((t1-1.0)/(t1+1.0));
		double ga = pow(t1*Gamma(t1),-1.0/t1);
		double gn = pow(RandomNG::uniform(0,1),1.0/t1)/ga;

		// relative energy loss (relative to E1)
		// double dp = gn-t2*gn*gn+ga*(ga*(ga-1.0)+t2)*gn*gn*gn;
		*/
		double dp = dEdx * rho * x * 100;	//(units: MeV/g/cm2 *g/cm3 *100cm)
		// cout <<"dp= "<<dp<<endl;

		if(dp > MAXDP)
		{
			dp = MAXDP;
		}

		// Adjust particle dp/p accordingly
		// p.dp() -= dp*(1.0+p.dp());

		// New particle (absolute) energy
		double E2 = E1 - dp * MeV;
		p.dp () = (E2 - E0) / E0;

		// Compute the random angle scatter assuming that the particle has
		// an average energy of (E+E0)/2 in the collimator
		double Eav = (E1 + E2) / 2.0;
		cout << "Eav:\t" << Eav << endl;
		double theta0 = 0.0136 * sqrt (t) * (1.0 + 0.038 * log (t)) / Eav;	// small-angle Coulomb scattering
		//double theta0 = 13.6e6 * sqrt (t) * (1.0 + 0.038 * log (t)) / Eav;	// small-angle Coulomb scattering
		// cout <<" E2 and theta0 "<<E2<<" "<<theta0<<endl;     
		pair < double, double >s = CoulombScatterp (x, theta0);
		p.x () += s.first;
		p.xp () += s.second;

		s = CoulombScatterp (x, theta0);
		p.y () += s.first;
		p.yp () += s.second;

		// if particle has lost 99% of its energy, make it walk the plank
		if (E2 < (E0 / 100.0))
		{
			cout << "I've lost all my energy" << endl;
			return 4;
		}

		return 0;			//keep particle
	}
	//  cout << "fell through without returning" << endl;
	// return 99;

	else
	{
		double r = RandomNG::uniform (0, 1) * (sig_pN_tot + sig_R);
		double sig_pN_el = sig_pN_tot - sig_pN_in;
		// subtract sigma from r, and check if it has gone negative
		if ((r -= sig_pN_el) < 0)
		{
			cout << "1" << endl;
			return 1;		//loose particle
		}
		else if ((r -= sig_pN_in) < 0)
		{
			cout << "2" << endl;
			return 2;		//loose particle
		}
		else if ((r -= sig_R) < 0)
		{
			cout << "3" << endl;
			return 3;		//loose particle
		}
	}

	//Return 0 in case of fall through? - should not get here.
	cout << "Bad scatter - did not return correctly" << endl;
	return 0;
}