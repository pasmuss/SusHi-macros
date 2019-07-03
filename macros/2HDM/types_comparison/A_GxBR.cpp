#include <TH3F.h>
#include <TH3.h>
#include <TGraph.h>
#include <TFile.h>
#include "TSystem.h"
#include <TCanvas.h>
#include <TLegend.h>


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

int main(){
  //	Histo with 2HDM results:
  string folder_path = "/nfs/dust/cms/user/asmusspa/private/CMSSW_9_2_15/src/SusHi/test_single_points_HA_allpoints_2016/rootFiles/";
  //string folder_path = "/nfs/dust/cms/user/asmusspa/private/CMSSW_9_2_15/src/SusHi/output_100PerJob/rootFiles/";
  vector<string> thdm_types {
    folder_path + "Histograms3D_type1_mA.root",
      folder_path + "Histograms3D_type2_mA.root",
      folder_path + "Histograms3D_type3_mA.root",
      folder_path + "Histograms3D_type4_mA.root",
      folder_path + "Histograms3D_type1_mH.root",
      folder_path + "Histograms3D_type2_mH.root",
      folder_path + "Histograms3D_type3_mH.root",
      folder_path + "Histograms3D_type4_mH.root"};
  //	Define mass points:
  vector<int> mAs{300,350,400,500,600,700,900};
  //	Define tnaBeta points:
  vector<double> tanBetas{10,20,30,40,50,60};
  //	Define cos(beta-alpha) points:
  vector<double> cosB_As{0};
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
	graphsTHDM[tanBetas.at(v)] = new TGraph(mAs.size());//map created for each combination of tan beta and cosB_a --> single graphs of map represent types
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
  else cout << "Neither A nor H selected. Doesn't work like that (GetTHDMHistos)." << endl;
}

double THDMBr(map< string, map<string,TH3F*> > histos, const string& type, const double& mA, const double& tanBeta, const double& cosB_A){
    
  string key_H = type+"_mH";
  string key_A = type+"_mA";

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
  
  cout << "total GxBR: " << XSxBRA+XSxBRH << endl;
  return XSxBRA+XSxBRH;
}


void DrawGraphs( map<double, TGraph*>& graphs2HDM, const string &type, const double& cosB_A){
  // setup canvas
  TCanvas can("can","can",600,600);
  can.SetLeftMargin(0.11);
  TLegend leg(0.6,0.7,0.9,0.9);
  //	leg.SetHeader(("2HDM(cos(#beta-#alpha) = " + to_string_with_precision(cosB_A,2) + ") vs MSSM(m_h^{mod+}, #mu = 200)").c_str());
  vector<int> colours = {1,2,3,4,6,9,12,28,46};
  // Loop over the tanBetas
  int i = 0;
  for (const auto& graph : graphs2HDM){
    int tanbetaint = (int)graph.first;
    graph.second->SetMarkerStyle(20 + i);
    graph.second->SetMarkerSize(1.1);
    graph.second->SetMarkerColor(colours.at(i));
    graph.second->SetTitle(";M_{A/H} [GeV]; #sigma(b#bar{b}A) #font[12]{B}(A#rightarrowbb) + #sigma(b#bar{b}H) #font[12]{B}(H#rightarrowbb) [pb]");
    graph.second->GetYaxis()->SetTitleOffset(1.7);
    leg.AddEntry(graph.second,("tan#beta = " + to_string(tanbetaint)).c_str(),"p");

    if(i==0){
      graph.second->Draw("AP");
      graph.second->GetYaxis()->SetRangeUser(1e-12,1e-3);
      graph.second->GetXaxis()->SetRangeUser(250,1150);
      graph.second->GetXaxis()->SetNdivisions(505);
    }
    else graph.second->Draw("Psame");
    can.SetLogy();
    can.Update();
    ++i;
  }
  leg.Draw();
  can.Print( (cmsswBase + "/src/Analysis/MssmHbb/macros/pictures/thdm_types_cosB_A-" + to_string_with_precision(cosB_A,3) + "_" + type + ".pdf").c_str() );
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
