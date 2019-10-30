/*
 * plotTanBetaLimits.cpp
 *
 *  Created on: 9 Mar 2017
 *  Author: shevchen
 *
 *  Macro to calculate and plot 2HDM limits
 *  as a function of mA and tanBeta for particluar
 *  value of cos(beta-alpha)
 */


#include <iostream>
#include <string>
#include <vector>

#include <TCanvas.h>
#include <TH2.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TPad.h>
#include <TFile.h>

#include "Analysis/MssmHbb/interface/THDMLimits.h"
#include "Analysis/MssmHbb/interface/Limit.h"
#include "Analysis/MssmHbb/interface/HbbStyleClass.h"
#include "Analysis/MssmHbb/interface/namespace_mssmhbb.h"

HbbStyle style;

using namespace std;
using namespace analysis::mssmhbb;
using namespace mssmhbb;

int main(int argc, const char** argv){

	PublicationStatus status = publication_status;
	style.setTDRstyle(status);
	//Prefix to the output
	string output_prefix = "13TeV_limits";
	//ATLAS results
	string ATLAS_results = "HIG-18-005";//"1502.04478";//"ATLAS-CONF-2017-055"
	//paths with results of the combine tool
	//string path2016 = cmsswBase + "/src/Analysis/MssmHbb/datacards/201712/13/unblinded/mhmodp_200/bias/Hbb.limits";
	string path2017SL = "/nfs/dust/cms/user/vagneria/2HDM/Hbb.limits";
	//value of cos(beta-alpha)
	double cB_A = 0.1;
	//Details of the 2HDM produciton
	string thdm_production = "FullRun_100PerJob_AllTypesAndBosons";
	// type of the 2hdm: type2 or type3 (=flipped); mhmodp_200/hMSSM should also work
	string thdm_type = "type2";
	if(argc != 1) {
		thdm_type = string(argv[1]);
	}
	string banch_name = "type2";
	if(thdm_type == "flipped") banch_name = "type3";
	else if (thdm_type == "lepton_specific") banch_name = "type4";
	AvailableScenarios scenario = AvailableScenariosFromString(thdm_type);
	string thdm_scans = "/nfs/dust/cms/user/asmusspa/public/CMSSW_9_2_15/src/Analysis/MssmHbb/SusHi/" + thdm_production + "/rootFiles/Histograms3D_" + banch_name + "_mA_mH.root";

	//higgs boson: H/A/both
	string boson = "both";
	string output = pictures_output + "Antonio/20190917_1707/";
//	string output = cmsswBase + "/src/Analysis/MssmHbb/macros/pictures/ParametricLimits/20171213/";
	if(!mssmhbb::blinded) output += "unblinded/";
	output += "2hdm/" + thdm_type + "/";
	CheckOutputDir(output);

	THDMLimits limits(mssmhbb::blinded,boson,125,700,0.7,60);
	limits.setScenario(scenario);
	limits.SetHiggsBoson(boson);
	limits.ReadCombineLimits(path2017SL);
	auto thdmLim = limits.Get2HDM_1D_Limits(thdm_scans,cB_A,"z");
	for(const auto& l: limits.getGxBrLimits()) cout<<"GxBR M: "<<l.getX()<<" exp = "<<l.getMedian()<<" +1G = "<<l.getPlus1G()<<" -1G = "<<l.getMinus1G()<<" +2G = "<<l.getPlus2G()<<" -2G = "<<l.getMinus2G()<<" Obs = "<<l.getObserved()<<endl;
	for(const auto& l: thdmLim) cout<<"M: "<<l.getX()<<" exp = "<<l.getMedian()<<" +1G = "<<l.getPlus1G()<<" -1G = "<<l.getMinus1G()<<" +2G = "<<l.getPlus2G()<<" -2G = "<<l.getMinus2G()<<" Obs = "<<l.getObserved()<<endl;
	limits.compareWithPrevious(ATLAS_results);

	// 2HDM tanBeta vs mA limits for cos(beta-alpha) = cB_A
	//TLegend leg_2HDM_tB(0.65,0.17,0.92,0.44);
	TLegend leg_2HDM_tB(0.35,0.74,0.93,0.87);
	output += thdm_type + "_" + boson + "_bosons_tanB_cB_A_" + output_prefix;
	limits.LimitPlotter(leg_2HDM_tB,output,"36.5 fb^{-1}",string(HbbStyle::axisTitleMAH().Data()),"tan#beta",true);
	output += "vs_ATLAS_" + ATLAS_results;
	output += "_zoomed";
	leg_2HDM_tB.Clear();
	limits.setXMin(125); limits.setYMin(10); limits.setYMax(100);
	limits.LimitPlotter(leg_2HDM_tB,output,"36.5 fb^{-1}",string(HbbStyle::axisTitleMAH().Data()),"tan#beta",true);

}
