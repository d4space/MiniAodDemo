// -*- C++ -*-
//
// Package:    MiniAodDemo/MiniAodDemo
// Class:      MiniAodDemo
// 
/**\class MiniAodDemo MiniAodDemo.cc MiniAodDemo/MiniAodDemo/plugins/MiniAodDemo.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  SangEun Lee
//         Created:  Tue, 07 Jul 2015 03:11:54 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "TTree.h"
#include <TVector2.h>

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

class MiniAodAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit MiniAodAnalyzer(const edm::ParameterSet&);
      ~MiniAodAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      edm::EDGetTokenT<pat::METCollection> slimMetToken_;
      edm::EDGetTokenT<pat::METCollection> patPFMetT1Token_;
      edm::EDGetTokenT<pat::METCollection> patPFMetT1TxyToken_;
      edm::EDGetTokenT<reco::VertexCollection> vtxToken_;

      edm::Service<TFileService> fs;
      TTree* OutTree;
      TVector2 t2_slimMet;
      TVector2 t2_patPFMetT1;
      TVector2 t2_patPFMetT1Txy;
      //Float_t type1pfmetPhi;
      Int_t  npv;
};

//
// constants, enums and typedefs
//
typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > LorentzVector;

//
// static data member definitions
//

//
// constructors and destructor
//
MiniAodAnalyzer::MiniAodAnalyzer(const edm::ParameterSet& iConfig):
  slimMetToken_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("slimMets"))),
  patPFMetT1Token_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("patPFMetT1"))),
  patPFMetT1TxyToken_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("patPFMetT1Txy"))),
  vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices")))
{
   //now do what ever initialization is needed
   usesResource("TFileService");


}


MiniAodAnalyzer::~MiniAodAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
MiniAodAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;



//#ifdef THIS_IS_AN_EVENT_EXAMPLE
//   Handle<ExampleData> pIn;
//   iEvent.getByLabel("example",pIn);
//#endif
   
//#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
//   ESHandle<SetupData> pSetup;
//   iSetup.get<SetupRecord>().get(pSetup);
//#endif
//
   Handle<pat::METCollection> slimMets;
   Handle<pat::METCollection> patPFMetT1s;
   Handle<pat::METCollection> patPFMetT1Txys;

   iEvent.getByToken(slimMetToken_, slimMets);
   iEvent.getByToken(patPFMetT1Token_, patPFMetT1s);
   iEvent.getByToken(patPFMetT1TxyToken_, patPFMetT1Txys);

   Handle<reco::VertexCollection> vertices;
   iEvent.getByToken(vtxToken_, vertices);
   // good vertex requirement
   if (vertices->empty()) return; // skip the event if no PV found
   const reco::Vertex &PV = vertices->front();
   npv = vertices->size();

   const pat::MET &slimMet       = slimMets->front();
   const pat::MET &patPFMetT1    = patPFMetT1s->front();
   const pat::MET &patPFMetT1Txy = patPFMetT1Txys->front();

   t2_slimMet       = TVector2(slimMet.px(),       slimMet.py());
   t2_patPFMetT1    = TVector2(patPFMetT1.px(),    patPFMetT1.py());
   t2_patPFMetT1Txy = TVector2(patPFMetT1Txy.px(), patPFMetT1Txy.py());
   //type1pfmet    = vtype1pfMET.Mod();
   //type1pfmetPhi = vtype1pfMET.Phi();

   // Fill Tree
   OutTree->Fill();

}


// ------------ method called once each job just before starting event loop  ------------
void 
MiniAodAnalyzer::beginJob()
{
  OutTree = fs->make<TTree>("Events", "Events");
  OutTree->Branch("t2_slimMet",       &t2_slimMet);   // Type-1 corrected PF MET
  OutTree->Branch("t2_patPFMetT1",    &t2_patPFMetT1);
  OutTree->Branch("t2_patPFMetT1Txy", &t2_patPFMetT1Txy);
  OutTree->Branch("npv",           &npv,           "npv/I");          // number of primary vertices

}

// ------------ method called once each job just after ending the event loop  ------------
void 
MiniAodAnalyzer::endJob() 
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
MiniAodAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MiniAodAnalyzer);
