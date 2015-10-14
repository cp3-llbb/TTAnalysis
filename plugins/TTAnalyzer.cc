#include <cp3_llbb/TTAnalysis/interface/Types.h>
#include <cp3_llbb/TTAnalysis/interface/Tools.h>
#include <cp3_llbb/TTAnalysis/interface/TTAnalyzer.h>
//#include <cp3_llbb/TTAnalysis/interface/NoZTTDileptonCategories.h>
//#include <cp3_llbb/TTAnalysis/interface/TTDileptonCategories.h>

#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include <cp3_llbb/Framework/interface/ElectronsProducer.h>
#include <cp3_llbb/Framework/interface/JetsProducer.h>
#include <cp3_llbb/Framework/interface/METProducer.h>

#include <Math/PtEtaPhiE4D.h>
#include <Math/LorentzVector.h>
#include <Math/VectorUtil.h>

// To access VectorUtil::DeltaR() more easily
using namespace ROOT::Math;

using namespace TTAnalysis;

void TTAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& setup, const ProducersManager& producers, const CategoryManager& categories) {
  
  // Initizalize vectors depending on IDs/WPs to the right lengths

  diLeptons_LepIDs.resize(LepLepID::Count);
  selectedJets_tightID_DRCut.resize(LepLepID::Count);
  diJets_DRCut.resize(LepLepID::Count);
  diLepDiJets_DRCut.resize(LepLepID::Count);
  diLepDiJetsMet_DRCut.resize(LepLepID::Count);
  diLepDiJetsMetNoHF_DRCut.resize(LepLepID::Count);
  
  selectedBJets_DRCut_BWPs_PtOrdered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BWP::Count));
  selectedBJets_DRCut_BWPs_CSVv2Ordered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BWP::Count));
  
  diBJets_DRCut_BBWPs_PtOrdered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BBWP::Count));
  diBJets_DRCut_BBWPs_CSVv2Ordered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BBWP::Count));
  diLepDiBJets_DRCut_BBWPs_PtOrdered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BBWP::Count));
  diLepDiBJets_DRCut_BBWPs_CSVv2Ordered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BBWP::Count));
  diLepDiBJetsMet_DRCut_BBWPs_PtOrdered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BBWP::Count));
  diLepDiBJetsMet_DRCut_BBWPs_CSVv2Ordered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BBWP::Count));
  diLepDiBJetsMetNoHF_DRCut_BBWPs_PtOrdered.resize(LepLepID::Count, std::vector<std::vector<uint8_t>>(BBWP::Count));
  
  ///////////////////////////
  //       ELECTRONS       //
  ///////////////////////////

  const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

  for(uint8_t ielectron = 0; ielectron < electrons.p4.size(); ielectron++){
    if( electrons.p4[ielectron].Pt() > m_electronPtCut && abs(electrons.p4[ielectron].Eta()) < m_electronEtaCut ){
      
      leptons.push_back( 
        Lepton(
          electrons.p4[ielectron], 
          ielectron, 
          electrons.charge[ielectron], 
          true, false,
          electrons.ids[ielectron][m_electronLooseIDName],
          electrons.ids[ielectron][m_electronMediumIDName],
          electrons.ids[ielectron][m_electronTightIDName],
          electrons.ids[ielectron][m_electronVetoIDName]
        )
      );
      
      if( electrons.ids[ielectron][m_electronVetoIDName] )
        vetoElectrons.push_back(ielectron);
      if( electrons.ids[ielectron][m_electronLooseIDName] )
        looseElectrons.push_back(ielectron);
      if( electrons.ids[ielectron][m_electronMediumIDName] )
        mediumElectrons.push_back(ielectron);
      if( electrons.ids[ielectron][m_electronTightIDName] )
        tightElectrons.push_back(ielectron);
    }
  }

  ///////////////////////////
  //       MUONS           //
  ///////////////////////////
  
  const MuonsProducer& muons = producers.get<MuonsProducer>("muons");

  for(uint8_t imuon = 0; imuon < muons.p4.size(); imuon++){
    if(muons.relativeIsoR04_withEA[imuon] < m_muonBaseIsoCut && muons.p4[imuon].Pt() > m_muonPtCut && abs(muons.p4[imuon].Eta()) < m_muonEtaCut ){
      
      leptons.push_back(
        Lepton(
          muons.p4[imuon], 
          imuon,
          muons.charge[imuon], 
          false, true,
          muons.isLoose[imuon],
          muons.isMedium[imuon],
          muons.isTight[imuon]
        )
      );

      if( muons.isLoose[imuon] )
        looseMuons.push_back(imuon);
      if( muons.isMedium[imuon] )
        mediumMuons.push_back(imuon);
      if( muons.isTight[imuon] )
        tightMuons.push_back(imuon);
    }
  }

  // Sort the leptons vector according to Pt
  std::sort(leptons.begin(), leptons.end(), [](const Lepton& a, const Lepton &b){ return b.p4.Pt() > a.p4.Pt(); });

  ///////////////////////////
  //       DILEPTONS       //
  ///////////////////////////

  for(uint8_t i1 = 0; i1 < leptons.size(); i1++){
    for(uint8_t i2 = i1 + 1; i2 < leptons.size(); i2++){
      const Lepton& l1 = leptons[i1];
      const Lepton& l2 = leptons[i2];

      DiLepton m_diLepton;

      m_diLepton.p4 = l1.p4 + l2.p4; 
      m_diLepton.idxs = std::make_pair(l1.idx, l2.idx); 
      m_diLepton.lidxs = std::make_pair(i1, i2); 
      m_diLepton.isElEl = l1.isEl && l2.isEl;
      m_diLepton.isElMu = l1.isEl && l2.isMu;
      m_diLepton.isMuEl = l1.isMu && l2.isEl;
      m_diLepton.isMuMu = l1.isMu && l2.isMu;
      m_diLepton.isOS = l1.charge != l2.charge;
      m_diLepton.isSF = m_diLepton.isElEl || m_diLepton.isMuMu;
      m_diLepton.lepIDs[LepLepID::LL] = l1.lepID[LepID::L] && l2.lepID[LepID::L];
      m_diLepton.lepIDs[LepLepID::LM] = l1.lepID[LepID::L] && l2.lepID[LepID::M];
      m_diLepton.lepIDs[LepLepID::ML] = l1.lepID[LepID::M] && l2.lepID[LepID::L];
      m_diLepton.lepIDs[LepLepID::LT] = l1.lepID[LepID::L] && l2.lepID[LepID::T];
      m_diLepton.lepIDs[LepLepID::TL] = l1.lepID[LepID::T] && l2.lepID[LepID::L];
      m_diLepton.lepIDs[LepLepID::MM] = l1.lepID[LepID::M] && l2.lepID[LepID::M];
      m_diLepton.lepIDs[LepLepID::MT] = l1.lepID[LepID::M] && l2.lepID[LepID::T];
      m_diLepton.lepIDs[LepLepID::TM] = l1.lepID[LepID::T] && l2.lepID[LepID::M];
      m_diLepton.lepIDs[LepLepID::TT] = l1.lepID[LepID::T] && l2.lepID[LepID::T];
      m_diLepton.DR = VectorUtil::DeltaR(l1.p4, l2.p4);
      m_diLepton.DEta = TTAnalysis::DeltaEta(l1.p4, l2.p4);
      m_diLepton.DPhi = VectorUtil::DeltaPhi(l1.p4, l2.p4);

      diLeptons.push_back(m_diLepton);
    }
  }

  for(uint8_t i = 0; i < diLeptons.size(); i++){
    const DiLepton& m_diLepton = diLeptons[i];
    
    for(const LepLepID::LepLepID& id: LepLepID::it){
      if(m_diLepton.lepIDs[id])
        diLeptons_LepIDs[id].push_back(i);
    }
    
  }

  ///////////////////////////
  //       JETS            //
  ///////////////////////////

  const JetsProducer& jets = producers.get<JetsProducer>("jets");

  // First find the jets passing kinematic cuts and tight jetID

  for(uint8_t ijet = 0; ijet < jets.p4.size(); ijet++){
      
    // Save the jets that pass the kinematic cuts
    if( abs(jets.p4[ijet].Eta()) < m_jetEtaCut && jets.p4[ijet].Pt() > m_jetPtCut){
      selectedJets.push_back(ijet);
        
      // Save the jets that pass the kinematic cuts and tight jetID
      if( abs(jets.p4[ijet].Eta()) < m_jetEtaCut && jets.p4[ijet].Pt() > m_jetPtCut && jetIDAccessor(jets, ijet, m_jetID)){
        selectedJets_tightID.push_back(ijet);
        
        // Save the jets that pass the kinematic cuts and tight jetID and DR(l,j)>cut using selected leptons, for each DiLepton ID pair
        for(const LepLepID::LepLepID& id: LepLepID::it){
          bool passDRCut(true);
          for(const DiLepton& m_diLepton: diLeptons){
            if( m_diLepton.lepIDs[id] && std::min(float(VectorUtil::DeltaR(jets.p4[ijet], leptons[m_diLepton.lidxs.first].p4)), float(VectorUtil::DeltaR(jets.p4[ijet], leptons[m_diLepton.lidxs.second].p4))) < m_jetDRleptonCut ){
              passDRCut = false;
              break;
            }
          }
          if(passDRCut)
            selectedJets_tightID_DRCut[id].push_back(ijet);
        }
      } // end selected + tightID
    } // end selected
  } // end for

  // Next, construct DiJets out of these (not accounting for minDRjl here)

  for(uint8_t j1 = 0; j1 < selectedJets_tightID.size(); j1++){
    for(uint8_t j2 = 0; j2 < selectedJets_tightID.size(); j2++){
      const uint8_t jet1 = selectedJets_tightID[j1];
      const uint8_t jet2 = selectedJets_tightID[j2];

      DiJet m_diJet; 
      m_diJet.p4 = jets.p4[jet1] + jets.p4[jet2];
      m_diJet.idxs = std::make_pair(jet1, jet2);
      
      m_diJet.DR = VectorUtil::DeltaR(jets.p4[jet1], jets.p4[jet2]);
      m_diJet.DPhi = VectorUtil::DeltaPhi(jets.p4[jet1], jets.p4[jet2]);
      m_diJet.DEta = DeltaEta(jets.p4[jet1], jets.p4[jet2]);
      
      m_diJet.CSVv2_WPs[BBWP::LL] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2L && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2L; 
      m_diJet.CSVv2_WPs[BBWP::LM] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2L && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2M; 
      m_diJet.CSVv2_WPs[BBWP::ML] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2M && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2L; 
      m_diJet.CSVv2_WPs[BBWP::LT] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2L && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2T; 
      m_diJet.CSVv2_WPs[BBWP::TL] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2T && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2L; 
      m_diJet.CSVv2_WPs[BBWP::MM] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2M && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2M; 
      m_diJet.CSVv2_WPs[BBWP::MT] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2M && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2T; 
      m_diJet.CSVv2_WPs[BBWP::TM] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2T && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2M; 
      m_diJet.CSVv2_WPs[BBWP::TT] = jets.getBTagDiscriminant(jet1, m_jetCSVv2Name) > m_jetCSVv2T && jets.getBTagDiscriminant(jet2, m_jetCSVv2Name) > m_jetCSVv2T;
      
      for(const auto& m_diLep: diLeptons){
        const float minDR = std::min( { 
            float(VectorUtil::DeltaR(jets.p4[jet1], leptons[m_diLep.lidxs.first].p4)), 
            float(VectorUtil::DeltaR(jets.p4[jet1], leptons[m_diLep.lidxs.second].p4)), 
            float(VectorUtil::DeltaR(jets.p4[jet2], leptons[m_diLep.lidxs.first].p4)), 
            float(VectorUtil::DeltaR(jets.p4[jet2], leptons[m_diLep.lidxs.second].p4))
          } );
        for(const LepLepID::LepLepID& id: LepLepID::it){
          if( minDR < m_diJet.minDRjl_lepIDs[id] && m_diLep.lepIDs[id] )
            m_diJet.minDRjl_lepIDs[id] = minDR;
        }

      }

      diJets.push_back(m_diJet); 
    }
  }

  // Save the DiJets which have minDRjl>cut, for each leptonID pair
  for(uint8_t i = 0; i < diJets.size(); i++){
    for(const LepLepID::LepLepID& id: LepLepID::it){
      if(diJets[i].minDRjl_lepIDs[id] > m_jetDRleptonCut)
        diJets_DRCut[id].push_back(i);
    }
  }

  ///////////////////////////
  //       B-JETS          //
  ///////////////////////////

  // Save b-jets one by one, this time taking into account minDRjl for different DiLepton ID pairs
  // Do it Pt- or CSVv2-ordered
  for(const LepLepID::LepLepID& id: LepLepID::it){
    for(uint8_t i = 0; i < selectedJets_tightID_DRCut[id].size(); i++){
      uint8_t ijet = selectedJets_tightID_DRCut[id][i];
      if(jets.getBTagDiscriminant(i, m_jetCSVv2Name) > m_jetCSVv2L)
        selectedBJets_DRCut_BWPs_PtOrdered[id][BWP::L].push_back(ijet);
      if(jets.getBTagDiscriminant(i, m_jetCSVv2Name) > m_jetCSVv2M)
        selectedBJets_DRCut_BWPs_PtOrdered[id][BWP::M].push_back(ijet);
      if(jets.getBTagDiscriminant(i, m_jetCSVv2Name) > m_jetCSVv2T)
        selectedBJets_DRCut_BWPs_PtOrdered[id][BWP::T].push_back(ijet);
    }
  }
  selectedBJets_DRCut_BWPs_CSVv2Ordered = selectedBJets_DRCut_BWPs_PtOrdered;
  for(const LepLepID::LepLepID& id: LepLepID::it){
    for(const BWP::BWP& wp: BWP::it){
      std::sort(selectedBJets_DRCut_BWPs_CSVv2Ordered[id][wp].begin(), selectedBJets_DRCut_BWPs_CSVv2Ordered[id][wp].end(), jetBTagDiscriminantSorter(jets, m_jetCSVv2Name));
    }
  }

  // Save di-b-jets, for each CSVv2 working point pair, taking into account minDRjl for each different DiLepton ID pair
  // Do it Pt- or CSVv2-ordered
  for(uint8_t i = 0; i < diJets.size(); i++){
    const DiJet& m_diJet = diJets[i];
    for(const LepLepID::LepLepID& id: LepLepID::it){
      for(const BBWP::BBWP& wp: BBWP::it){
        if(m_diJet.CSVv2_WPs[wp] && m_diJet.minDRjl_lepIDs[id] > m_jetDRleptonCut)
          diBJets_DRCut_BBWPs_PtOrdered[id][wp].push_back(i);
      }
    }
  }
  diBJets_DRCut_BBWPs_CSVv2Ordered = diBJets_DRCut_BBWPs_PtOrdered;
  for(const LepLepID::LepLepID& id: LepLepID::it){
    for(const BBWP::BBWP& wp: BBWP::it)
      std::sort(diBJets_DRCut_BBWPs_CSVv2Ordered[id][wp].begin(), diBJets_DRCut_BBWPs_CSVv2Ordered[id][wp].end(), diJetBTagDiscriminantSorter(jets, m_jetCSVv2Name, diJets));
  }

  ///////////////////////////
  //    EVENT VARIABLES    //
  ///////////////////////////
  
  // leptons-(b-)jets

  int diLepDiJetCounter(0);

  for(uint8_t dilep = 0; dilep < diLeptons.size(); dilep++){
    const DiLepton& m_diLepton = diLeptons[dilep];
    
    for(uint8_t dijet = 0; dijet < diJets.size(); dijet++){
      const DiJet& m_diJet =  diJets[dijet];
      
      DiLepDiJet m_diLepDiJet(m_diLepton, dilep, m_diJet, dijet);

      m_diLepDiJet.minDRjl = std::min( {
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.second])),
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.second]))
          } );
      m_diLepDiJet.maxDRjl = std::max( {
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.second])),
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaR(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.second]))
          } );
      m_diLepDiJet.minDEtajl = std::min( {
          DeltaEta(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.first]),
          DeltaEta(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.second]),
          DeltaEta(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.first]),
          DeltaEta(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.second])
          } );
      m_diLepDiJet.maxDEtajl = std::max( {
          DeltaEta(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.first]),
          DeltaEta(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.second]),
          DeltaEta(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.first]),
          DeltaEta(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.second])
          } );
      m_diLepDiJet.minDPhijl = std::min( {
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.second])),
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.second]))
          } );
      m_diLepDiJet.maxDPhijl = std::max( {
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.first].p4, jets.p4[m_diJet.idxs.second])),
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.first])),
          float(VectorUtil::DeltaPhi(leptons[m_diLepton.lidxs.second].p4, jets.p4[m_diJet.idxs.second]))
          } );

      diLepDiJets.push_back(m_diLepDiJet);

      for(const LepLepID::LepLepID& id: LepLepID::it){
        if(m_diLepton.lepIDs[id] && m_diLepDiJet.minDRjl > m_jetDRleptonCut){
          diLepDiJets_DRCut[id].push_back(diLepDiJetCounter);
          for(const BBWP::BBWP& wp: BBWP::it){
            if(m_diJet.CSVv2_WPs[wp])
              diLepDiBJets_DRCut_BBWPs_PtOrdered[id][wp].push_back(diLepDiJetCounter);
          }
        }
      }

      diLepDiJetCounter++;
    }
  }

  diLepDiBJets_DRCut_BBWPs_CSVv2Ordered = diLepDiBJets_DRCut_BBWPs_PtOrdered;
  for(const LepLepID::LepLepID& id: LepLepID::it){
    for(const BBWP::BBWP& wp: BBWP::it)
      std::sort(diLepDiBJets_DRCut_BBWPs_CSVv2Ordered[id][wp].begin(), diLepDiBJets_DRCut_BBWPs_CSVv2Ordered[id][wp].end(), diJetBTagDiscriminantSorter(jets, m_jetCSVv2Name, diLepDiJets));
  }

      
  // leptons-(b-)jets-MET

  const METProducer &met = producers.get<METProducer>("met");
  const METProducer &noHFmet = producers.get<METProducer>("nohf_met");
  
  for(uint8_t i = 0; i < diLepDiJets.size(); i++){
    // Using regular MET
    DiLepDiJetMet m_diLepDiJetMet(diLepDiJets[i], i, met.p4);
    
    m_diLepDiJetMet.minDR_l_Met = std::min(
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMet.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMet.diLepton->lidxs.second].p4, met.p4))
        );
    m_diLepDiJetMet.maxDR_l_Met = std::max(
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMet.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMet.diLepton->lidxs.second].p4, met.p4))
        );
    m_diLepDiJetMet.minDEta_l_Met = std::min(
        DeltaEta(leptons[m_diLepDiJetMet.diLepton->lidxs.first].p4, met.p4),
        DeltaEta(leptons[m_diLepDiJetMet.diLepton->lidxs.second].p4, met.p4)
        );
    m_diLepDiJetMet.maxDEta_l_Met = std::max(
        DeltaEta(leptons[m_diLepDiJetMet.diLepton->lidxs.first].p4, met.p4),
        DeltaEta(leptons[m_diLepDiJetMet.diLepton->lidxs.second].p4, met.p4)
        );
    m_diLepDiJetMet.minDPhi_l_Met = std::min(
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMet.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMet.diLepton->lidxs.second].p4, met.p4))
        );
    m_diLepDiJetMet.maxDPhi_l_Met = std::max(
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMet.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMet.diLepton->lidxs.second].p4, met.p4))
        );

    m_diLepDiJetMet.minDR_j_Met = std::min(
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMet.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMet.diJet->idxs.second], met.p4))
        );
    m_diLepDiJetMet.maxDR_j_Met = std::max(
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMet.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMet.diJet->idxs.second], met.p4))
        );
    m_diLepDiJetMet.minDEta_j_Met = std::min(
        DeltaEta(jets.p4[m_diLepDiJetMet.diJet->idxs.first], met.p4),
        DeltaEta(jets.p4[m_diLepDiJetMet.diJet->idxs.second], met.p4)
        );
    m_diLepDiJetMet.maxDEta_j_Met = std::max(
        DeltaEta(jets.p4[m_diLepDiJetMet.diJet->idxs.first], met.p4),
        DeltaEta(jets.p4[m_diLepDiJetMet.diJet->idxs.second], met.p4)
        );
    m_diLepDiJetMet.minDPhi_j_Met = std::min(
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMet.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMet.diJet->idxs.second], met.p4))
        );
    m_diLepDiJetMet.maxDPhi_j_Met = std::max(
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMet.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMet.diJet->idxs.second], met.p4))
        );

    diLepDiJetsMet.push_back(m_diLepDiJetMet);

    // Using noHF MET
    DiLepDiJetMet m_diLepDiJetMetNoHF(diLepDiJets[i], i, noHFmet.p4, true);
    
    m_diLepDiJetMetNoHF.minDR_l_Met = std::min(
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.second].p4, met.p4))
        );
    m_diLepDiJetMetNoHF.maxDR_l_Met = std::max(
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaR(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.second].p4, met.p4))
        );
    m_diLepDiJetMetNoHF.minDEta_l_Met = std::min(
        DeltaEta(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.first].p4, met.p4),
        DeltaEta(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.second].p4, met.p4)
        );
    m_diLepDiJetMetNoHF.maxDEta_l_Met = std::max(
        DeltaEta(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.first].p4, met.p4),
        DeltaEta(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.second].p4, met.p4)
        );
    m_diLepDiJetMetNoHF.minDPhi_l_Met = std::min(
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.second].p4, met.p4))
        );
    m_diLepDiJetMetNoHF.maxDPhi_l_Met = std::max(
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.first].p4, met.p4)),
        float(VectorUtil::DeltaPhi(leptons[m_diLepDiJetMetNoHF.diLepton->lidxs.second].p4, met.p4))
        );

    m_diLepDiJetMetNoHF.minDR_j_Met = std::min(
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.second], met.p4))
        );
    m_diLepDiJetMetNoHF.maxDR_j_Met = std::max(
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaR(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.second], met.p4))
        );
    m_diLepDiJetMetNoHF.minDEta_j_Met = std::min(
        DeltaEta(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.first], met.p4),
        DeltaEta(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.second], met.p4)
        );
    m_diLepDiJetMetNoHF.maxDEta_j_Met = std::max(
        DeltaEta(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.first], met.p4),
        DeltaEta(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.second], met.p4)
        );
    m_diLepDiJetMetNoHF.minDPhi_j_Met = std::min(
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.second], met.p4))
        );
    m_diLepDiJetMetNoHF.maxDPhi_j_Met = std::max(
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.first], met.p4)),
        float(VectorUtil::DeltaPhi(jets.p4[m_diLepDiJetMetNoHF.diJet->idxs.second], met.p4))
        );

    diLepDiJetsMet.push_back(m_diLepDiJetMetNoHF);
     
    for(const LepLepID::LepLepID& id: LepLepID::it){
      if(m_diLepDiJetMet.diLepton->lepIDs[id] && m_diLepDiJetMet.diJet->minDRjl_lepIDs[id] > m_jetDRleptonCut){
        diLepDiJetsMet_DRCut[id].push_back(i);
        for(const BBWP::BBWP& wp: BBWP::it){
          if(m_diLepDiJetMet.diJet->CSVv2_WPs[wp])
            diLepDiBJetsMet_DRCut_BBWPs_PtOrdered[id][wp].push_back(i);
        }
      }
      if(m_diLepDiJetMetNoHF.diLepton->lepIDs[id] && m_diLepDiJetMetNoHF.diJet->minDRjl_lepIDs[id] > m_jetDRleptonCut){
        diLepDiJetsMetNoHF_DRCut[id].push_back(i);
        for(const BBWP::BBWP& wp: BBWP::it){
          if(m_diLepDiJetMetNoHF.diJet->CSVv2_WPs[wp])
            diLepDiBJetsMetNoHF_DRCut_BBWPs_PtOrdered[id][wp].push_back(i);
        }
      }
    }
  }
  
  diLepDiBJetsMet_DRCut_BBWPs_CSVv2Ordered = diLepDiBJetsMet_DRCut_BBWPs_PtOrdered; 
  for(const LepLepID::LepLepID& id: LepLepID::it){
    for(const BBWP::BBWP& wp: BBWP::it)
      std::sort(diLepDiBJetsMet_DRCut_BBWPs_CSVv2Ordered[id][wp].begin(), diLepDiBJetsMet_DRCut_BBWPs_CSVv2Ordered[id][wp].end(), diJetBTagDiscriminantSorter(jets, m_jetCSVv2Name, diLepDiJetsMet));
  }
  
  diLepDiBJetsMetNoHF_DRCut_BBWPs_CSVv2Ordered = diLepDiBJetsMetNoHF_DRCut_BBWPs_PtOrdered; 
  for(const LepLepID::LepLepID& id: LepLepID::it){
    for(const BBWP::BBWP& wp: BBWP::it)
      std::sort(diLepDiBJetsMetNoHF_DRCut_BBWPs_CSVv2Ordered[id][wp].begin(), diLepDiBJetsMetNoHF_DRCut_BBWPs_CSVv2Ordered[id][wp].end(), diJetBTagDiscriminantSorter(jets, m_jetCSVv2Name, diLepDiJetsMet));
  }

  ///////////////////////////
  //       TRIGGER         //
  ///////////////////////////

  ///////////////////////////
  //       GEN INFO        //
  ///////////////////////////

}

void TTAnalyzer::registerCategories(CategoryManager& manager, const edm::ParameterSet& config) {
  /*manager.new_category<TTMuMuCategory>("mumu", "Category with leading leptons as two muons", config);
  manager.new_category<TTElElCategory>("elel", "Category with leading leptons as two electrons", config);
  manager.new_category<TTMuElCategory>("muel", "Category with leading leptons as muon, electron", config);
  manager.new_category<TTElMuCategory>("elmu", "Category with leading leptons as electron, muon", config);
  
  manager.new_category<NoZTTMuMuCategory>("noZmumu", "Category with leading leptons as two muons, excluding the Z peak", config);
  manager.new_category<NoZTTElElCategory>("noZelel", "Category with leading leptons as two electrons, excluding the Z peak", config);*/
}

#include <FWCore/PluginManager/interface/PluginFactory.h>
DEFINE_EDM_PLUGIN(ExTreeMakerAnalyzerFactory, TTAnalyzer, "tt_analyzer");
