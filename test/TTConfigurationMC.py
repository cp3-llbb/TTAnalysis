import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
from cp3_llbb.Framework import Framework

process = Framework.create(False, eras.Run2_25ns, '74X_mcRun2_asymptotic_v2', cms.PSet(
    tt = cms.PSet(
        type = cms.string('tt_analyzer'),
        prefix = cms.string('tt_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            electronPtCut = cms.untracked.double(20),
            electronEtaCut = cms.untracked.double(2.5),
            electronVetoIDName = cms.untracked.string('cutBasedElectronID-Spring15-25ns-V1-standalone-veto'),
            electronLooseIDName = cms.untracked.string('cutBasedElectronID-Spring15-25ns-V1-standalone-loose'),
            electronMediumIDName = cms.untracked.string('cutBasedElectronID-Spring15-25ns-V1-standalone-medium'),
            electronTightIDName = cms.untracked.string('cutBasedElectronID-Spring15-25ns-V1-standalone-tight'),
            
            muonPtCut = cms.untracked.double(20),
            muonEtaCut = cms.untracked.double(2.4),
            muonBaseIsoCut = cms.untracked.double(.20), # Loose cut recommended for dilepton analysis

            #MllBaseCutSF = cms.untracked.double(20), # Mll cut for same-flavour opposite-sign lepton pairs
            #MllBaseCutDF = cms.untracked.double(0), # Mll cut for different-flavour opposite-sign lepton pairs
            
            jetPtCut = cms.untracked.double(30),
            jetEtaCut = cms.untracked.double(2.5),
            jetPUID = cms.untracked.double(-9999999),
            jetDRleptonCut = cms.untracked.double(0.3),
            #jetID = cms.untracked.string('tight'), # not tightLeptonVeto since DeltaR(l,j) cut should be enough
            jetCSVv2Name = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            jetCSVv2L = cms.untracked.double(0.605),
            jetCSVv2M = cms.untracked.double(0.89),
            jetCSVv2T = cms.untracked.double(0.97),

            hltDRCut = cms.untracked.double(0.3), # DeltaR cut for trigger matching
            hltPtCut = cms.untracked.double(99999), #Delta(Pt)/Pt cut for trigger matching
            ),
        #categories_parameters = cms.PSet(
        #    mll_cut = cms.untracked.double(20),
        #    mll_ZVetoCut_low = cms.untracked.double(86),
        #    mll_ZVetoCut_high = cms.untracked.double(116)
        #    ),
        )
    ), redoJEC=True
    )

Framework.schedule(process, ['tt'])

process.source.fileNames = cms.untracked.vstring(
        'file:///home/fynu/sbrochet/storage/MINIAODSIM/TTJets_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8_Asympt25ns_MCRUN2_74_V9_reduced.root'
        )

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(2000))
