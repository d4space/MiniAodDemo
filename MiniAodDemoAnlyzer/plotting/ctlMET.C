#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TVector2.h>
#include <TF1.h>
#include <TMath.h>

void ctlMET(const TString inputFileName = "Wenu_p_select.root") {

  //
  // Settings
  //
  Int_t NVTXBINS = 35; // 70 for Wenu_p, 35 for Wmunu_p
  //
  // Setup input ntuple
  //
  TFile* inputFile = new TFile(inputFileName);
  TTree* inputTree = (TTree*)inputFile->Get("MiniAodAna/Events");

  //
  // Declare variables to read in ntuple
  //
  Int_t nVtx, nEvents;

  TVector2 *vtype1pfMET=0, *vrawpfMET=0, *vgenMET=0;
  TVector2 *t2_slimMet=0, *t2_patPFMetT1=0, *t2_patPFMetT1Txy=0;

  inputTree->SetBranchAddress("npv",         &nVtx);        // number of vertices
//  inputTree->SetBranchAddress("nEvents",      &nEvents);     // number of events
  //inputTree->SetBranchAddress("t2_type1pfmet",  &vtype1pfMET); // type-1 corrected pf MET
//  inputTree->SetBranchAddress("vrawpfMET",    &vrawpfMET);   // raw pf MET
//  inputTree->SetBranchAddress("vgenMET",      &vgenMET);     // generated MET
  inputTree->SetBranchAddress("t2_slimMet",       &t2_slimMet);
  inputTree->SetBranchAddress("t2_patPFMetT1",    &t2_patPFMetT1);
  inputTree->SetBranchAddress("t2_patPFMetT1Txy", &t2_patPFMetT1Txy);
           
  //
  // Declare histograms
  //
  TH1D *hSlimMet  = new TH1D("hSlimMet","",100,0,150);
        hSlimMet->SetStats(0);
        hSlimMet->SetLineColor(1);
  TH1D *hpatPFMetT1  = new TH1D("patPFMetT1","",100,0,150);
        hpatPFMetT1->SetStats(0);
        hpatPFMetT1->SetLineColor(1);
  TH1D *hpatPFMetT1Txy  = new TH1D("patPFMetT1Txy","",100,0,150);
        hpatPFMetT1Txy->SetStats(0);
        hpatPFMetT1Txy->SetLineColor(1);
  TH1D *htype1phi = new TH1D("htype1phi","",100,-3.5,3.5);
        htype1phi->SetStats(0);
        htype1phi->SetLineColor(1);
  TH1D *htype1corr  = new TH1D("htype1corr","",100,0,150);
        htype1corr->SetStats(0);
        htype1corr->SetLineColor(4);
  TH1D *htype1phicorr = new TH1D("htype1phicorr","",100,-3.5,3.5);
        htype1phicorr->SetStats(0);
        htype1phicorr->SetLineColor(4);
  TH2D *hmetx = new TH2D("hmetx","MET_{x} v. Number of Vertices",NVTXBINS,0,NVTXBINS,100,-150,150);
        hmetx->GetXaxis()->SetTitle("Number of vertices");
        hmetx->GetYaxis()->SetTitle("MET_{x} [GeV]");
  TH2D *hmety = new TH2D("hmety","MET_{y} v. Number of Vertices",NVTXBINS,0,NVTXBINS,100,-150,150);
        hmety->GetXaxis()->SetTitle("Number of vertices");
        hmety->GetYaxis()->SetTitle("MET_{y} [GeV]");

  //
  // Make legend
  //
  TLegend *leg = new TLegend(0.4181034,0.6758475,0.6954023,0.8135593,NULL,"brNDC");
  leg->SetTextFont(62);
  leg->SetTextSize(0.03330866);
  leg->SetLineColor(1);
  leg->SetLineStyle(1);
  leg->SetLineWidth(1);
  leg->SetFillColor(0);
  leg->SetFillStyle(1001);
  leg->SetBorderSize(0);

  leg->AddEntry(hSlimMet,"Type 1 Corrected PF","l");
  leg->AddEntry(htype1corr,"Type 1 + xy Shift Corrected PF","l");

  Int_t totalEvents=0;

  for(int jentry=0;jentry<inputTree->GetEntries();jentry++) {
    inputTree->GetEntry(jentry);
    totalEvents += nEvents;

    //
    // Fill histograms
    //
    hmetx->Fill(nVtx,t2_slimMet->Px());
    hmety->Fill(nVtx,t2_slimMet->Py());
  }

  cout << "totalEvents is " << totalEvents << endl;

  // Loop through nVtx bins and find the mean value of metx and mety in each bin
  // Plot nVtx v. mean values of metx/mety in a separate histogram (1 for metx and 1 for mety)
  TH1D* hmetx_proj = new TH1D();
  TH1D* hmety_proj = new TH1D();
  Double_t meanmetx, meanmety;
  TH2D* hmetxfit = new TH2D("hmetxfit","MET_{x} v. Number of vertices",NVTXBINS,0,NVTXBINS,100,-25,5);
        hmetxfit->GetXaxis()->SetTitle("Number of vertices");
        hmetxfit->GetYaxis()->SetTitle("<MET_{x}> [GeV]");
        hmetxfit->SetMarkerSize(21);
  TH2D* hmetyfit = new TH2D("hmetyfit","MET_{y} v. Number of vertices",NVTXBINS,0,NVTXBINS,100,-25,5);
        hmetyfit->GetXaxis()->SetTitle("Number of vertices");
        hmetyfit->GetYaxis()->SetTitle("<MET_{y}> [GeV]");
        hmetyfit->SetMarkerSize(21);
  for(int jbin=1;jbin<hmetx->GetNbinsX()+1;jbin++) {
    hmetx_proj = hmetx->ProjectionY("metx_proj",jbin,jbin+1,"");
    hmety_proj = hmety->ProjectionY("mety_proj",jbin,jbin+1,"");
    meanmetx = hmetx_proj->GetMean();
    meanmety = hmety_proj->GetMean();
    hmetxfit->Fill(jbin,meanmetx);
    hmetyfit->Fill(jbin,meanmety);
  }

  // Fit each of these with a line to calculate the appropriate correction factor for an event with the given number of vertices
  // Estimation of initial values for parameters can be streamlined (don't hardcode in the 49 and 50)
  TF1 *flinex = new TF1("flinex","[0]+[1]*x",0,NVTXBINS);
  Double_t xpar0=0, xpar1=0;
  xpar0 = hmetxfit->ProjectionY("",2,3,"")->GetMean(); // mean of metx for events with 1 vertex (minimum)
  xpar1 = (hmetxfit->ProjectionY("",49,50,"")->GetMean()-xpar0)/hmetxfit->GetNbinsX(); // slope estimate from 2 outermost points
  flinex->SetParameter(0,xpar0);
  flinex->SetParameter(1,xpar1);
  hmetxfit->Fit(flinex);
  TF1 *fliney = new TF1("fliney","[0]+[1]*x",0,NVTXBINS);
  Double_t ypar0=0, ypar1=0;
  ypar0 = hmetyfit->ProjectionY("",2,3,"")->GetMean(); // mean of mety for events with 1 vertex (minimum)
  ypar1 = (hmetyfit->ProjectionY("",NVTXBINS-1,NVTXBINS,"")->GetMean()-ypar0)/NVTXBINS; // slope estimate from 2 outermost points
  fliney->SetParameter(0,ypar0);
  fliney->SetParameter(1,ypar1);
  hmetyfit->Fit(fliney);

  std::cout << std::endl;
  std::cout << "x fit parameter initial values: " << xpar0 << "," << xpar1 << std::endl;
  std::cout << "x fit parameters final values: " << flinex->GetParameter(0) << "," << flinex->GetParameter(1) << std::endl;
  std::cout << std::endl;
  std::cout << "y fit parameter initial values: " << ypar0 << "," << ypar1 << std::endl;
  std::cout << "y fit parameters final values: " << fliney->GetParameter(0) << "," << fliney->GetParameter(1) << std::endl;
  std::cout << std::endl;

  // Correct the MET
  Double_t flinex0=flinex->GetParameter(0), flinex1=flinex->GetParameter(1);
  Double_t fliney0=fliney->GetParameter(0), fliney1=fliney->GetParameter(1);
  Double_t type1x=0, type1y=0;
  Double_t type1phi=0, rawphi=0, genphi=0;
  Double_t corrMETx=0, corrMETy=0, corrMET=0, corrMETphi=0;
  for(int kentry=0;kentry<inputTree->GetEntries();kentry++) {
    inputTree->GetEntry(kentry);

    type1x = t2_slimMet->Px();
    type1y = t2_slimMet->Py();

    type1phi = t2_slimMet->Phi();
    if(type1phi>TMath::Pi()) type1phi -= 2*TMath::Pi();
    //rawphi = vrawpfMET->Phi();
    //if(rawphi>TMath::Pi())   rawphi   -= 2*TMath::Pi();
    //genphi = vgenMET->Phi();
    //if(genphi>TMath::Pi())   genphi   -= 2*TMath::Pi();

    corrMETx = type1x-flinex0-flinex1*nVtx;
    corrMETy = type1y-fliney0-fliney1*nVtx;
    corrMET  = TMath::Sqrt(corrMETx*corrMETx + corrMETy*corrMETy);
    corrMETphi = TMath::ATan2(corrMETy,corrMETx);

    //
    // Fill histograms
    //
    hSlimMet      ->Fill(t2_slimMet->Mod());
    hpatPFMetT1   ->Fill(t2_patPFMetT1->Mod());
    hpatPFMetT1Txy->Fill(t2_patPFMetT1Txy->Mod());
    htype1phi->Fill(type1phi);
    htype1corr->Fill(corrMET);
    htype1phicorr->Fill(corrMETphi);
  }

  //
  // Save plots
  //
  TLegend *leg_all = new TLegend(0.4181034,0.6758475,0.6954023,0.8135593,NULL,"brNDC");
  leg_all->SetTextFont(62);
  leg_all->SetTextSize(0.03330866);
  leg_all->SetLineColor(1);
  leg_all->SetLineStyle(1);
  leg_all->SetLineWidth(1);
  leg_all->SetFillColor(0);
  leg_all->SetFillStyle(1001);
  leg_all->SetBorderSize(0);

  leg_all->AddEntry(hSlimMet,"Slimmed","l");
  leg_all->AddEntry(hpatPFMetT1,"patPFMetT1","l");
  leg_all->AddEntry(hpatPFMetT1Txy,"patPFMetT1Txy","l");

  TCanvas* tc_allmet = new TCanvas();
  tc_allmet->cd();
  hSlimMet->Draw();
  hpatPFMetT1->Draw("same");
  hpatPFMetT1Txy->Draw("same");
  leg_all->Draw("same");
  tc_allmet->Print("allMet.png");

  TCanvas* cmet = new TCanvas();
  cmet->cd();
  hSlimMet->Draw("same");
  htype1corr->Draw("same");
  leg->Draw("same");
  cmet->Print("met.png");
  cmet->Close();
  TCanvas* cmetphi = new TCanvas();
  cmetphi->cd();
  htype1phi->Draw("same");
  htype1phicorr->Draw("same");
  leg->Draw("same");
  cmetphi->Print("metphi.png");
//  cmetphi->Close();
  TCanvas* cmetx = new TCanvas();
  cmetx->cd();
  hmetx->Draw();
  cmetx->Print("metx.png");
  cmetx->Close();
  TCanvas* cmety = new TCanvas();
  cmety->cd();
  hmety->Draw();
  cmety->Print("mety.png");
  cmety->Close();
  TCanvas* cmetxfit = new TCanvas();
  cmetxfit->cd();
  hmetxfit->Draw();
  cmetxfit->Print("metxfit.png");
  cmetxfit->Close();
  TCanvas* cmetyfit = new TCanvas();
  cmetyfit->cd();
  hmetyfit->Draw();
  cmetyfit->Print("metyfit.png");
  cmetyfit->Close();
}
