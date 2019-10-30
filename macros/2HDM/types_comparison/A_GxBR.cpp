#include <TH3F.h>
#include <TH3.h>
#include <TGraph.h>
#include <TFile.h>
#include "TSystem.h"
#include <TCanvas.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TH1.h>
#include <TH1F.h>
#include <TAxis.h>
#include <TGaxis.h>

#include <string>
#include <vector>

// xsection estimation for MSSM
#include "Analysis/MssmHbb/macros/signal/mssm_xs_tools.h"
#include "Analysis/MssmHbb/macros/signal/mssm_xs_tools.C"
// Tools
#include "Analysis/MssmHbb/interface/utilLib.h"

using namespace std;

const auto cmsswBase = static_cast<std::string>(gSystem->Getenv("CMSSW_BASE"));

void CompareSigmaXBr(const vector<int>&, const vector<double>&, const vector<double>&, const vector<string>&);
double MssmBr(mssm_xs_tools& my, const double& mA, const double& tanBeta);
double THDMBr(map< string, map<string,TH3F*> > histos, const string& type, const double& mA, const double& tanBeta, const double& cosB_A);
void DrawGraphs( map<double, TGraph*>& graphs2HDM, const string &type, const double& cosB_A);
void GetTHDMHistos(TFile& f,map<string,TH3F*>& histos, const string& s);
void DrawCMSInfo(const char* energy, const char* status);

int main(){
  //	Histo with 2HDM results:
  //string folder_path = "/nfs/dust/cms/user/asmusspa/private/CMSSW_9_2_15/src/SusHi/test_single_points_HA_allpoints_2016/rootFiles/";
  string folder_path = "/nfs/dust/cms/user/asmusspa/public/CMSSW_9_2_15/src/Analysis/MssmHbb/SusHi/FullRun_100PerJob_AllTypesAndBosons/rootFiles/";
  vector<string> thdm_types {
    //folder_path + "Histograms3D_type1_mA.root",
      folder_path + "Histograms3D_type2_mA.root",
      folder_path + "Histograms3D_type3_mA.root",
	//folder_path + "Histograms3D_type4_mA.root",
	//folder_path + "Histograms3D_type1_mH.root",
      folder_path + "Histograms3D_type2_mH.root",
      folder_path + "Histograms3D_type3_mH.root",
	//folder_path + "Histograms3D_type4_mH.root",
	//folder_path + "Histograms3D_type1_mh.root",
      folder_path + "Histograms3D_type2_mh.root",
      folder_path + "Histograms3D_type3_mh.root",
	//folder_path + "Histograms3D_type4_mh.root"
	};
  //	Define mass points:
  vector<int> mAs{125,130,140,160,180,200,250,300,350,400,450,500,600,700/*,800,900,1000,1200,1400,1600*/};
  //	Define tnaBeta points:
  vector<double> tanBetas{10,20,30,40,50,60};
  //	Define cos(beta-alpha) points:
  vector<double> cosB_As{0,-0.3,0.3,-0.5,0.5,-0.1,0.1,0.8,-0.8};
  CompareSigmaXBr(mAs,tanBetas,cosB_As,thdm_types);
  return 0;
}

void CompareSigmaXBr(const vector<int> &mAs, const vector<double> &tanBetas, const vector<double> &cosB_As, const vector<string>& thdm_types){
  vector<string> types;//typen_mX
  map< string, map<string,TH3F*> > thdm_maps;
  map<string,TFile*> tfiles;
  int k =0;
  for(const auto& thdm : thdm_types){
    string thdm_type_string = thdm.substr(thdm.find("type"),8);
    cout << "type string: " << thdm_type_string << endl;
    string thdm_model = thdm.substr(thdm.find("type"),5);
    tfiles[thdm_type_string] = new TFile(thdm.c_str(),"read");//assign file to map; key = type+boson
    GetTHDMHistos(*tfiles[thdm_type_string],thdm_maps[thdm_type_string],thdm_type_string);
    if ( find(types.begin(), types.end(), thdm_model) == types.end() ) types.push_back(thdm_model);
    for (unsigned int i = 0; i<types.size(); i++) cout << types.at(i) << endl;
    ++k;
  }//thdm-types
  for(const auto& cosB_A : cosB_As){
    for(const auto& type : types){
      map<double,TGraph*> graphsTHDM;
      for(unsigned int v = 0; v != tanBetas.size(); ++v){
	graphsTHDM[tanBetas.at(v)] = new TGraph(mAs.size());
	int j = 0;
	for(const auto& mA : mAs){
	  ++j;
	  double value = THDMBr(thdm_maps,type,mA,tanBetas.at(v),cosB_A);
	  cout<<"M_A/H: "<<mA<<" GxBR: "<<value<<endl;
	  graphsTHDM[tanBetas.at(v)]->SetPoint(j,mA,value);
	}//mA
      }//types
      std::cout<<"DRAW"<<std::endl;
      DrawGraphs(graphsTHDM,type,cosB_A);
    }//type
  }//cos(B-A)
}//CompareSigmaBr

void GetTHDMHistos(TFile& f,map<string,TH3F*>& histos, const string& s){    
  CheckZombie(f);
  int bosontypeA = s.find("mA");
  int bosontypeH = s.find("mH");
  int bosontypeh = s.find("mh");
  //for A
  if ( bosontypeA > 0){
    CheckZombieObjectInTFile(f,"xs_bbA");
    histos["xs_bbA"] = (TH3F*) f.Get("xs_bbA");
    CheckZombieObjectInTFile(f,"br_Abb");
    histos["br_Abb"] = (TH3F*) f.Get("br_Abb");
  }
  //for H
  else if ( bosontypeH > 0){
    CheckZombieObjectInTFile(f,"xs_bbH");
    histos["xs_bbH"] = (TH3F*) f.Get("xs_bbH");
    CheckZombieObjectInTFile(f,"br_Hbb");
    histos["br_Hbb"] = (TH3F*) f.Get("br_Hbb");
  }
  //for h
  else if ( bosontypeh > 0){
    CheckZombieObjectInTFile(f,"xs_bbh");
    histos["xs_bbh"] = (TH3F*) f.Get("xs_bbh");
    CheckZombieObjectInTFile(f,"br_hbb");
    histos["br_hbb"] = (TH3F*) f.Get("br_hbb");
  }

  else cout << "Neither A nor H or h selected. Doesn't work like that (GetTHDMHistos)." << endl;
}

double THDMBr(map< string, map<string,TH3F*> > histos, const string& type, const double& mA, const double& tanBeta, const double& cosB_A){
    
  string key_H = type+"_mH";
  string key_A = type+"_mA";
  string key_h = type+"_mh";

  //for A
  cout << "br calc mA: " << mA << ", tanBeta: " << tanBeta << ", cosB_A: " << cosB_A <<  endl;
  double sigmaBBA = histos[key_A]["xs_bbA"]->Interpolate(mA,tanBeta,cosB_A);
  double BrAbb = histos[key_A]["br_Abb"]->Interpolate(mA,tanBeta,cosB_A);
  std::cout << "sigma: " << sigmaBBA << ", br: " << BrAbb << std::endl;
  double XSxBRA = sigmaBBA*BrAbb;
  std::cout << "GxBR: " << XSxBRA << std::endl;
  //for H
  std::cout << "br calc mH: " << mA << ", tanBeta: " << tanBeta << ", cosB_A: " << cosB_A <<  std::endl;
  double sigmaBBH = histos[key_H]["xs_bbH"]->Interpolate(mA,tanBeta,cosB_A);
  double BrHbb = histos[key_H]["br_Hbb"]->Interpolate(mA,tanBeta,cosB_A);
  std::cout << "sigma: " << sigmaBBH << ", br: " << BrHbb << std::endl;
  double XSxBRH = sigmaBBH*BrHbb;
  std::cout << "GxBR: " << XSxBRH << std::endl;
  //for h
  if (mA == 125) std::cout << "br calc mH/A: " << mA << ", tanBeta: " << tanBeta << ", cosB_A: " << cosB_A <<  std::endl;
  double sigmaBBh = histos[key_h]["xs_bbh"]->Interpolate(mA,tanBeta,cosB_A);
  double Brhbb = histos[key_h]["br_hbb"]->Interpolate(mA,tanBeta,cosB_A);
  if (mA == 125) std::cout << "sigma: " << sigmaBBh << ", br: " << Brhbb << std::endl;
  double XSxBRh = sigmaBBh*Brhbb;
  if (mA == 125) std::cout << "GxBR: " << XSxBRh << std::endl;

  if (mA == 125) cout << "total GxBR: " << XSxBRA+XSxBRH+XSxBRh << endl;
  else cout << "total GxBR: " << XSxBRA+XSxBRH << endl;
  if (mA == 125) return XSxBRA+XSxBRH+XSxBRh;
  else return XSxBRA+XSxBRH;
}


void DrawGraphs( map<double, TGraph*>& graphs2HDM, const string &type, const double& cosB_A){
  // setup canvas
  TCanvas can("can","can",600,600);
  can.SetLeftMargin(0.11);
  TLegend leg(0.11,0.755,0.9,0.9);
  leg.SetTextSize(0.03);
  leg.SetNColumns(3);
  leg.SetTextFont(62);
  string typechar;
  if (type == "type1") typechar = "type I";
  else if (type == "type2") typechar = "type II";
  else if (type == "type3") typechar = "flipped";
  else if (type == "type4") typechar = "lepton specific";
  leg.SetHeader( ("2HDM " + typechar + ", cos(#beta - #alpha) = " + to_string_with_precision(cosB_A,2)  + ", m_{h} = 125 GeV").c_str() );
  //	leg.SetHeader(("2HDM(cos(#beta-#alpha) = " + to_string_with_precision(cosB_A,2) + ") vs MSSM(m_h^{mod+}, #mu = 200)").c_str());
  vector<int> colours = {1,2,3,4,6,9,12,28,46};
  // Loop over the tanBetas
  int i = 0;
  for (const auto& graph : graphs2HDM){
    int tanbetaint = (int)graph.first;
    graph.second->SetMarkerStyle(20 + i);
    graph.second->SetMarkerSize(1.1);
    graph.second->SetMarkerColor(colours.at(i));
    leg.AddEntry(graph.second,("tan#beta = " + to_string(tanbetaint)).c_str(),"p");

    if(i==0){
      graph.second->Draw("AP");
      graph.second->GetHistogram()->GetYaxis()->ChangeLabel(-1,-1,0,-1,-1,-1,"");
      //graph.second->SetTitle(";M_{A/H} [GeV]; #sigma(b#bar{b}A) #font[12]{B}(A#rightarrowbb) + #sigma(b#bar{b}H) #font[12]{B}(H#rightarrowbb) [pb]");
      graph.second->SetTitle(";M_{A/H} [GeV]; #sigma(b#bar{b}#varphi) #font[12]{B}(#varphi#rightarrowb#bar{b}) [pb], #varphi = h,H,A");
      graph.second->GetYaxis()->SetTitleOffset(1.7);
      graph.second->GetYaxis()->CenterTitle();
      if (type == "type1") graph.second->GetYaxis()->SetRangeUser(1e-1,1e2);
      else if (type == "type2") graph.second->GetYaxis()->SetRangeUser(1e-4,1e6);
      else if (type == "type3") graph.second->GetYaxis()->SetRangeUser(1e-4,1e6);
      else if (type == "type4") graph.second->GetYaxis()->SetRangeUser(1e-13,1e-4);
      graph.second->GetXaxis()->SetRangeUser(100,750);
      graph.second->GetXaxis()->SetNdivisions(505);
    }
    else graph.second->Draw("Psame");
    can.SetLogy();
    can.SetTicks(1,1);
    can.Update();
    ++i;
  }
  leg.Draw();
  DrawCMSInfo("(13 TeV)","Private Simulation");
  can.Print( (cmsswBase + "/src/Analysis/MssmHbb/macros/pictures/hHA_thdm_types_cosB_A-" + to_string_with_precision(cosB_A,3) + "_" + type + ".pdf").c_str() );
  can.Print( (cmsswBase + "/src/Analysis/MssmHbb/macros/pictures/hHA_thdm_types_cosB_A-" + to_string_with_precision(cosB_A,3) + "_" + type + ".root").c_str() );
}

void DrawCMSInfo(const char* energy, const char* status){
  TPaveText* CMS = new TPaveText(0.105,0.86,0.2,0.95,"NDC");
  CMS->SetBorderSize(0);
  CMS->SetFillStyle(0);
  CMS->SetTextAlign(11);
  CMS->SetTextSize (0.04);
  CMS->SetTextColor(1);
  CMS->SetTextFont (62);
  CMS->AddText("CMS");
  CMS->Draw();

  TPaveText* STATUS = new TPaveText(0.19,0.86,0.4,0.95,"NDC");
  STATUS->SetBorderSize(0);
  STATUS->SetFillStyle(0);
  STATUS->SetTextAlign(11);
  STATUS->SetTextSize (0.03);
  STATUS->SetTextColor(1);
  STATUS->SetTextFont (52);
  STATUS->AddText(status);
  STATUS->Draw();

  TPaveText* ENERGY = new TPaveText(0.8,0.863,0.909,0.95,"NDC");
  ENERGY->SetBorderSize(0);
  ENERGY->SetFillStyle(0);
  ENERGY->SetTextAlign(31);
  ENERGY->SetTextSize (0.02);
  ENERGY->SetTextColor(1);
  ENERGY->SetTextFont (42);
  ENERGY->AddText(energy);
  ENERGY->Draw();

  TPaveText* rootSucks = new TPaveText(0.04,0.8,0.105,0.92,"NDC");
  rootSucks -> SetFillStyle(1001);
  rootSucks -> SetFillColor(kWhite);
  rootSucks -> SetBorderSize(0);
  rootSucks -> Draw();
}

double MssmBr(mssm_xs_tools& my,
	      const double& mA,
	      const double& tanBeta) {
  double sigmaBBA = my.bbHSantander_A(mA,tanBeta);
  //    double sigmaBBH = my.bbHSantander_H(mA,tanBeta);

  double BrAbb = my.br_Abb(mA,tanBeta);
  //    double BrHbb = my.br_Hbb(mA,tanBeta);

  double totXSec = sigmaBBA*BrAbb;// + sigmaBBH*BrHbb;

  return totXSec;
}
