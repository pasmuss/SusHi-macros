/*
 * plotTanBetaLimits.cpp
 *
 *  Created on: 9 Mar 2017
 *  Author: shevchen
 *
 *  Macro to calculate and plot 2HDM limits
 *  as a function of cos(beta-alpha) and tan(beta)
 *  for particluar value of mA
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

// void Plot_finale_1(const std::vector<Limit>& limits,
// 		TLegend& leg,
// 		const bool&,
// 		const std::string& output,
// 		const float& yMin,
// 		const float& yMax,
// 		const float& xMin,
// 		const float& xMax,
// 		const std::string& Lumi,
// 		const std::string& xtitle,
// 		const std::string& ytitle,
// 		const std::string& thdm_type,
// 		const bool& logY);
// //TGraph GetAtlasZhll_flipped();
// 
// TGraph GetAtlasZhll_flipped_lower_left();
// TGraph GetAtlasZhll_flipped_upper_left();
// TGraph GetAtlasZhll_flipped_lower_right();
// TGraph GetAtlasZhll_flipped_upper_right();
// 
// TGraph GetAtlasZhll_type2_left();
// TGraph GetAtlasZhll_type2_lower_right();
// TGraph GetAtlasZhll_type2_upper_right();
// 
// void AtlasMeasurementsStyle(TGraph &gr);

using namespace std;
using namespace analysis::mssmhbb;
using namespace mssmhbb;

int main(int argc, const char **argv){

	PublicationStatus status = publication_status;
	status = PRIVATE;
	style.setTDRstyle(status);
	//Prefix to the output
	string output_prefix = "13TeV_limits";
	//ATLAS results
	string ATLAS_results = "ATLAS-CONF-2017-055";//"1502.04478";//
	//paths with results of the combine tool
	string path2016 = cmsswBase + "/src/Analysis/MssmHbb/datacards/201712/13/unblinded/mhmodp_200/bias/Hbb.limits";
	//value of cos(beta-alpha)
	double mass = 1100;
	//Details of the 2HDM produciton
	string thdm_production = "production_cosB_A_-1_1_tanB_0p5-100_COMBINATION";
	// type of the 2hdm: type2 or type3
	string thdm_type = "type2";
	if(argc != 1) {
		thdm_type = string(argv[1]);
	}
	string banch_name = thdm_type;
	if(thdm_type == "flipped") banch_name = "type3";
	else if (thdm_type == "lepton_specific") banch_name = "type4";
	AvailableScenarios scenario = AvailableScenariosFromString(thdm_type);

	string thdm_scans = "/nfs/dust/cms/user/shevchen/SusHiScaner/output/" + thdm_production + "/rootFiles/Histograms3D_" + banch_name + "_mA_mH.root";

	//higgs boson: H/A/both
	string boson = "both";
//	string output = pictures_output + "/ParametricLimits/20171213/";
//	string output = cmsswBase + "/src/Analysis/MssmHbb/macros/pictures/ParametricLimits/20171213/";
	string output = cmsswBase + "/src/Analysis/MssmHbb/macros/pictures/Thesis/limits/";
	if(!mssmhbb::blinded) output += "unblinded/";
	output += "2hdm/" + thdm_type + "/";
	CheckOutputDir(output);

	THDMLimits limits(mssmhbb::blinded,boson,-1,1,0,100);
	limits.setScenario(scenario);
	limits.setXMax(1);
	limits.SetHiggsBoson(boson);
	limits.ReadCombineLimits(path2016);
	auto lol = limits.Get2HDM_1D_Limits(thdm_scans,mass,"x");

	vector<Limit> GBR2016 = limits.getGxBrLimits();
	TH3D GxBR_2hdm_3D;
	TH2D GxBR_2hdm_mA;
	vector<Limit> THDM_limits;
	for(const auto& l : GBR2016){
		cout<<"M: "<<l.getX()<<" exp = "<<l.getMedian()<<" +1G = "<<l.getPlus1G()<<" -1G = "<<l.getMinus1G()<<" +2G = "<<l.getPlus2G()<<" -2G = "<<l.getMinus2G()<<" Obs = "<<l.getObserved()<<endl;
		if(l.getX() == mass){
			std::cout<<"Make limits for mass = "<<l.getX()<<std::endl;
			GxBR_2hdm_mA = limits.Get2HDM_GxBR_2D(thdm_scans,mass,"x");
			if(thdm_type!="type1" && thdm_type!="type4") limits.Get2HDM_Limits(GxBR_2hdm_mA,l);
		}
	}

	TLegend leg(0.35,0.74,0.93,0.87); // In case of
	output += thdm_type + "_" + boson + "_bosons_mA" + to_string((int)mass) + "_"+ output_prefix;
	limits.compareWithPrevious("");
	if(thdm_type!="type1" && thdm_type!="type4"){
		// plot full cos(b-a) range
		limits.LimitPlotter(leg,output,"35.7 fb^{-1}","cos(#beta-#alpha)","tan#beta",false);
		if(mass == 300){
			// plot comparison to ATLAS
			if(ATLAS_results == "ATLAS-CONF-2017-055"){
				limits.setXMin(-0.5); limits.setXMax(0.5);
				limits.setYMin(4);limits.setYMax(50);
//				limits.setXMax(0.9); limits.setYMax(50);
			} else if(ATLAS_results == "1502.04478"){
				limits.setXMin(-0.8); limits.setYMin(4);
				limits.setXMax(0.8); limits.setYMax(50);
				
			}
			output += "vs_ATLAS_" + ATLAS_results;
			limits.compareWithPrevious(ATLAS_results);
//			leg.SetX1NDC(-0.4); leg.SetX2NDC(0.9);
//			leg.SetY1NDC(0.3); leg.SetY2NDC(0.5);
			leg.Clear();
			output += "_zoomed";
//			limits.setXMin(-0.8); limits.setYMin(4);
//			limits.setXMax(0.8); limits.setYMax(50);
			// plot zoomed cos(b-a) range
			if(thdm_type == "type2"){
				if(ATLAS_results == "1502.04478"){
				limits.setXMin(-0.9); limits.setYMin(4);
				limits.setXMax(0.9); limits.setYMax(50);
//				leg.SetX1NDC(0.4); leg.SetX2NDC(0.75);
//				leg.SetY1NDC(0.27); leg.SetY2NDC(0.52);
				}
			}

			limits.LimitPlotter(leg,output,"35.7 fb^{-1}","cos(#beta-#alpha)","tan#beta",true);

		}
	}

}
