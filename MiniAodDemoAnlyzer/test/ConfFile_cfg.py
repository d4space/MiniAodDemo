import FWCore.ParameterSet.Config as cms

process = cms.Process("Met")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.destinations = ['cout', 'cerr']
process.MessageLogger.cerr.FwkReport.reportEvery = 1000


process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        #'file:myfile.root'
	'file:/u/user/salee/MET/Mat9117/CMSSW_7_5_0_pre6/src/PhysicsTools/PatAlgos/test/testminiAOD.root'
    )
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string('miniAodDemo.root')
    )

process.MiniAodAna = cms.EDAnalyzer('MiniAodAnalyzer',
    vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    slimMets = cms.InputTag("slimmedMETs"),
    patPFMetT1 = cms.InputTag("patPFMetT1"),
    patPFMetT1Txy = cms.InputTag("patPFMetT1Txy"),
)


process.p = cms.Path(process.MiniAodAna)
