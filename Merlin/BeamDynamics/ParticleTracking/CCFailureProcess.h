/////////////////////////////////////////////////////////////////////////
//
// Merlin C++ Class Library for Charged Particle Accelerator Simulations
//
// Class library version 5.01 (2015)
//
// Copyright: see Merlin/copyright.txt
//
// Created:		21.09.15 Haroon Rafique + Andrea Santamaria Garcia (CERN)
// Modified:
// Last Edited: 22.09.15 HR + ASG
//
/////////////////////////////////////////////////////////////////////////
#ifndef CCFailureProcess_h
#define CCFailureProcess_h 1

#include "merlin_config.h"

#include "AcceleratorModel/AcceleratorModel.h"
#include "AcceleratorModel/StdComponent/CrabMarker.h"

#include "BeamDynamics/ParticleTracking/ParticleBunchProcess.h"
#include "BeamDynamics/ParticleTracking/ParticleBunch.h"

#include "RingDynamics/LatticeFunctions.h"

namespace ParticleTracking
{

// A Physics Process based on Andrea Santamaria Garcia's simple crab kick model, and allowing for a voltage or phase failure
// Note that in order to use process this we must inject a gaussian bunch immediately before a set of upstream CCs
// We assume that both sets of CCs are in use (horizontal @ CMS, and vertical @ ATLAS), and have been placed in the TFS table as CRABMARKER elements
// Currently the phase advances calculated in MADX are used as MERLIN doesn't store this information in the LatticeFunctionTable

class CCFailureProcess : public ParticleBunchProcess
{
public:

// Standard ParticleBunchProcess functions

	//	Constructor
	CCFailureProcess (int priority, int mode, AcceleratorModel* model, LatticeFunctionTable* twiss);
	CCFailureProcess (int priority, int mode, AcceleratorModel* model, LatticeFunctionTable* twiss, double freq, double crossing, double phase);
	CCFailureProcess (int priority, int mode, AcceleratorModel* model, LatticeFunctionTable* twiss, double freq, double crossing, double phase, int non_fail_turn, int fail_turn);

	//	Initialise this process with the specified Bunch. If
	//	bunch is not a ParticleBunch object, the process becomes
	//	inactive.
	virtual void InitialiseProcess (Bunch& bunch);

	//	Sets the current accelerator component.
	virtual void SetCurrentComponent (AcceleratorComponent& component);

	//	Preform the process for the specified step ds.
	virtual void DoProcess (double ds);

	//	Returns the current maximum step length for this process.
	virtual double GetMaxAllowedStepSize() const;

// Process specific functions

	// Returns the M12 and M22 transfer matrix elements between two points
	//~ virtual pair<double,double> CalcM_12_22( bool horizontal, int element2, int element1 = 0);

	//Return M12 or M22 of twiss matrix
	//for voltage the phase advance is between CC and IP, for x/y kicks it is between the start and end of the CC
	virtual double CalcM_12(int start, int end, double deltamu, bool horizontal);
	virtual double CalcM_22(int start, int end, double deltamu, bool horizontal);
	virtual double CalcM_12(int start, int end, bool horizontal);
	virtual double CalcM_22(int start, int end, bool horizontal);

	// Returns the phase advance in x and y for element
	virtual pair<double,double> CalcMu(int element);

	// Returns the phase advance in x and y from element1 to element2
	virtual pair<double,double> CalcDeltaMu(int element1, int element2);

	// Returns the voltage V1 (pre IP)
	virtual double CalcV1(double M12);
	virtual double CalcV1(double deltamu, int n1, int n2, bool horizontal);

	// Returns the voltage V2 (post IP)
	virtual double CalcV2(double V1, double M22);

	// Performs the particle kick
	virtual void ApplyPreCCKick(PSvector &p, double V, double M12, bool horizontal);
	virtual void ApplyPostCCKick(PSvector &p, double V, double M12, bool horizontal);

	// Switch on/off Failure
	virtual void SetFailureOnOff(bool onoff)
	{
		failure_on = onoff;
		cout << "CCFailure::Failure_on = " << failure_on << endl;
	}

	// Select number of turns for failure
	virtual void SetFailureTurns(int ft)
	{
		fail_turns = ft;
		cout << "CCFailure::SetFailureTurns = " << fail_turns << endl;
	}
	virtual void SetNonFailureTurns(int nft)
	{
		non_fail_turns = nft;
		cout << "CCFailure::SetNonFailureTurns = " << non_fail_turns << endl;
	}

	// Select which plane to fail (horizontal = ATLAS, vertical = CMS)
	virtual void SetFailurePlanes(bool hor, bool ver)
	{
		ATLAS_on = hor;
		CMS_on = ver;
		cout << "CCFailure::SetFailurePlanes: ATLAS = " << ATLAS_on << ", CMS = " << CMS_on << endl;
	}

private:
	// Data Members for Class Attributes
	AcceleratorModel* AccModelCC;
	LatticeFunctionTable* TwissCC;

	bool ATLAS_on;
	bool CMS_on;

	bool Horizontal_CC;	//Horizontal	=1 	Vertical	=0
	bool upstream;		//upstream		=1 	downstream	=0
	bool ATLAS; 		//ATLAS			=1 	CMS			=0

	//Use these to find if we have made a complete turn (in terms of CCs)
	bool IP1_up;
	bool IP1_down;
	bool IP5_up;
	bool IP5_down;
	int IP1_up_count;
	int IP1_down_count;
	int IP5_up_count;
	int IP5_down_count;


	//Store upstream voltages
	double Atlas_Upstream_V1[4];
	double CMS_Upstream_V1[4];
	//Store upstream element numbers
	double Atlas_Upstream_elno[4];
	double CMS_Upstream_elno[4];
	//Store upstream deltamu
	double Atlas_Upstream_deltamu[4];
	double CMS_Upstream_deltamu[4];
	// store M12
	double Atlas_Upstream_M12[4];
	double CMS_Upstream_M12[4];

	double Gamma_p;
	double Beta_p;
	double omega;		//crab cavity frequency
	double theta;		//half crossing angle
	double phi_s;		//crab phase
	double EnergyCC;	//Beam energy

	//Failure variables
	int non_fail_turns;
	int fail_turns;
	bool failure_on;

	int Turn;
	int n;			//number of crabs pre/post IP
	int testn;
};


} // end namespace ParticleTracking
#endif
