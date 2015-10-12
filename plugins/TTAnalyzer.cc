#include <cp3_llbb/TTAnalysis/interface/Types.h>
#include <cp3_llbb/TTAnalysis/interface/Tools.h>
#include <cp3_llbb/TTAnalysis/interface/GenStatusFlags.h>
#include <cp3_llbb/TTAnalysis/interface/TTAnalyzer.h>
#include <cp3_llbb/TTAnalysis/interface/NoZTTDileptonCategories.h>
#include <cp3_llbb/TTAnalysis/interface/TTDileptonCategories.h>

#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include <cp3_llbb/Framework/interface/ElectronsProducer.h>
#include <cp3_llbb/Framework/interface/JetsProducer.h>
#include <cp3_llbb/Framework/interface/METProducer.h>
#include <cp3_llbb/Framework/interface/GenParticlesProducer.h>

// To access VectorUtil::DeltaR() more easily
using namespace ROOT::Math;

using namespace TTAnalysis;

void TTAnalyzer::analyze(const edm::Event& event, const edm::EventSetup& setup, const ProducersManager& producers, const CategoryManager& categories) {

    ///////////////////////////
    //       ELECTRONS       //
    ///////////////////////////

    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    for(uint8_t ielectron = 0; ielectron < electrons.p4.size(); ielectron++){
      if( electrons.p4[ielectron].Pt() > m_electronPtCut && abs(electrons.p4[ielectron].Eta()) < m_electronEtaCut ){
        Lepton m_lepton(electron.p4[ielectron], ielectron, electron.charge[ielectron], true, false);
        
        if( electrons.ids[ielectron][m_electronVetoIDName] ){
          vetoElectrons.push_back(ielectron);
          m_lepton.lID["veto"] = true;
        }
        if( electrons.ids[ielectron][m_electronLooseIDName] ){
          looseElectrons.push_back(ielectron);
          m_lepton.lID["loose"] = true;
        }
        if( electrons.ids[ielectron][m_electronMediumIDName] ){
          mediumElectrons.push_back(ielectron);
          m_lepton.lID["medium"] = true;
        }
        if( electrons.ids[ielectron][m_electronTightIDName] ){
          tightElectrons.push_back(ielectron);
          m_lepton.lID["tight"] = true;
        }

        lepton.push_back(m_lepton);

        /*if( electrons.ids[ielectron][m_electronSelectedIDName] ){
            selectedElectrons.push_back(ielectron);
            m_leptons.push_back( { electrons.p4[ielectron], ielectron, true, false } );
        }*/
      }
    }

    ///////////////////////////
    //       MUONS           //
    ///////////////////////////
    
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");

    for(uint8_t imuon = 0; imuon < muons.p4.size(); imuon++){
      if(muons.relativeIsoR04_withEA[imuon] < m_muonBaseIsoCut && muons.p4[imuon].Pt() > m_muonPtCut && abs(muons.p4[imuon].Eta()) < m_muonEtaCut ){
        Lepton m_lepton(muon.p4[imuon], imuon, muon.charge[imuon], false, true);

        if( muons.isLoose[imuon] ){
          looseMuons.push_back(imuon);
          m_lepton.lID["loose"] = true;
        }
        if( muons.isMedium[imuon] ){
          mediumMuons.push_back(imuon);
          m_lepton.lID["medium"] = true;
        }
        if( muons.isTight[imuon] ){
          tightMuons.push_back(imuon);
          m_lepton.lID["tight"] = true;
        }

        leptons.push_back(m_lepton);
      }

      /*if(muonIDAccessor(muons, imuon, m_muonSelectedID) && muons.relativeIsoR04_withEA[imuon] < m_muonSelectedIsoCut && muons.p4[imuon].Pt() > m_muonPtCut && abs(muons.p4[imuon].Eta()) < m_muonEtaCut ){
          selectedMuons.push_back(imuon);
          m_leptons.push_back( { muons.p4[imuon], imuon, false, true } );
      }*/
    }

    // Sort the leptons vector according to Pt
    std::sort(leptons.begin(), leptons.end(), [](const Lepton& a, const Lepton &b){ return b.p4.Pt() > a.p4.Pt(); });
    /*for(const Lepton& lepton: m_leptons){
      lepton_p4.push_back(lepton.p4);
      lepton_idx.push_back(lepton.idx);
      lepton_isMu.push_back(lepton.isMu);
      lepton_isEl.push_back(lepton.isEl);
    }*/

    ///////////////////////////
    //       DILEPTONS       //
    ///////////////////////////

    for(uint8_t i1 = 0; i1 < leptons.size(); i1++){
      for(uint8_t i2 = i1 + 1; i2 < leptons.size(); i2++){
        const Lepton& l1 = leptons[i1];
        const Lepton& l2 = leptons[i2];

        diLeptons.push_back( diLepton(
            l1.p4 + l2.p4, 
            std::make_pair(l1.idx, l2.idx), 
            std::make_pair(i1, i2), 
            l1.isEl && l2.isEl,
            l1.isEl && l2.isMu,
            l1.isMu && l2.isEl,
            l1.isMu && l2.isMu,
            l1.charge != l2.charge,
            (l1.isEl && l2.isEl) || (l1.isMu && l2.isMu),
            VectorUtil::DeltaR(l1.p4, l2.p4),
            TTAnalysis::DeltaEta(l1.p4, l2.p4),
            VectorUtil::DeltaPhi(l1.p4, l2.p4)
            ) );
      }
    }
    
    

    // First find the highest-Pt opposite-charge pairs for each couple of flavours, on selected objets only
    // Already apply a cut on Mll
    
    /*leadingSelectedElEl = std::make_pair(-1, -1);
    leadingSelectedElMu = std::make_pair(-1, -1);
    leadingSelectedMuEl = std::make_pair(-1, -1);
    leadingSelectedMuMu = std::make_pair(-1, -1);

    float tempSumPtElEl(0), tempSumPtElMu(0), tempSumPtMuEl(0), tempSumPtMuMu(0); 

    // El-El
    for(uint8_t iele1 = 0; iele1 < selectedElectrons.size(); iele1++){
        for(uint8_t iele2 = iele1+1; iele2 < selectedElectrons.size(); iele2++){
            if( electrons.charge[ selectedElectrons[iele1] ] * electrons.charge[ selectedElectrons[iele2] ] < 0 && (electrons.p4[ selectedElectrons[iele1] ] + electrons.p4[ selectedElectrons[iele2] ]).M() > m_MllBaseCutSF ){
                leadingSelectedElEl = std::make_pair(iele1, iele2);
                tempSumPtElEl = electrons.p4[ selectedElectrons[iele1] ].Pt() + electrons.p4[ selectedElectrons[iele2] ].Pt(); 
                break;
            }
        }
        if(leadingSelectedElEl.first >= 0)
            break;
    }

    // El-Mu
    for(uint8_t iele = 0; iele < selectedElectrons.size(); iele++){
        for(uint8_t imu = 0; imu < selectedMuons.size(); imu++){
            if( electrons.p4[ selectedElectrons[iele] ].Pt() > muons.p4[ selectedMuons[imu] ].Pt() && electrons.charge[ selectedElectrons[iele] ] * muons.charge[ selectedMuons[imu] ] < 0 && (electrons.p4[ selectedElectrons[iele] ] + muons.p4[ selectedMuons[imu] ]).M() > m_MllBaseCutDF){
                leadingSelectedElMu = std::make_pair(iele, imu);
                tempSumPtElMu = electrons.p4[ selectedElectrons[iele] ].Pt() + muons.p4[ selectedMuons[imu] ].Pt(); 
                break;
            }
        }
        if(leadingSelectedElMu.first >= 0)
            break;
    }
    
    // Mu-El
    for(uint8_t imu = 0; imu < selectedMuons.size(); imu++){
        for(uint8_t iele = 0; iele < selectedElectrons.size(); iele++){
            if( electrons.p4[ selectedElectrons[iele] ].Pt() < muons.p4[ selectedMuons[imu] ].Pt() && electrons.charge[ selectedElectrons[iele] ] * muons.charge[ selectedMuons[imu] ] < 0 && (electrons.p4[ selectedElectrons[iele] ] + muons.p4[ selectedMuons[imu] ]).M() > m_MllBaseCutDF){
                leadingSelectedMuEl = std::make_pair(imu, iele);
                tempSumPtMuEl = electrons.p4[ selectedElectrons[iele] ].Pt() + muons.p4[ selectedMuons[imu] ].Pt(); 
                break;
            }
        }
        if(leadingSelectedMuEl.first >= 0)
            break;
    }

    // Mu-Mu
    for(uint8_t imu1 = 0; imu1 < selectedMuons.size(); imu1++){
        for(uint8_t imu2 = imu1+1; imu2 < selectedMuons.size(); imu2++){
            if( muons.charge[ selectedMuons[imu1] ] * muons.charge[ selectedMuons[imu2] ] < 0 && (muons.p4[ selectedMuons[imu1] ] + muons.p4[ selectedMuons[imu2] ]).M() > m_MllBaseCutSF){
                leadingSelectedMuMu = std::make_pair(imu1, imu2);
                tempSumPtMuMu = muons.p4[ selectedMuons[imu1] ].Pt() + muons.p4[ selectedMuons[imu2] ].Pt(); 
                break;
            }
        }
        if(leadingSelectedMuMu.first >= 0)
            break;
    }

    // Next, we define the selected lepton pair as the highest Sum(Pt) pair

    if( (tempSumPtElEl > 0) && (tempSumPtElEl > tempSumPtElMu) && (tempSumPtElEl > tempSumPtMuEl) && (tempSumPtElEl > tempSumPtMuMu) ){
      m_diLepton = { 
        electrons.p4[leadingSelectedElEl.first] + electrons.p4[leadingSelectedElEl.second], 
        std::make_pair(leadingSelectedElEl.first, leadingSelectedElEl.second), // electron/muon indices
        std::make_pair(-1, -1), // lepton indices to be retrieved later
        true, false, false, false
        };
    }else if( (tempSumPtElMu > 0) && (tempSumPtElMu > tempSumPtElEl) && (tempSumPtElMu > tempSumPtMuEl) && (tempSumPtElMu > tempSumPtMuMu) ){
      m_diLepton = { 
        electrons.p4[leadingSelectedElMu.first] + muons.p4[leadingSelectedElMu.second], 
        std::make_pair(leadingSelectedElMu.first, leadingSelectedElMu.second),
        std::make_pair(-1, -1),
        false, true, false, false
        };
    }else if( (tempSumPtMuEl > 0) && (tempSumPtMuEl > tempSumPtElEl) && (tempSumPtMuEl > tempSumPtElMu) && (tempSumPtMuEl > tempSumPtMuMu) ){
      m_diLepton = { 
        muons.p4[leadingSelectedMuEl.first] + electrons.p4[leadingSelectedMuEl.second], 
        std::make_pair(leadingSelectedMuEl.first, leadingSelectedMuEl.second),
        std::make_pair(-1, -1),
        false, false, true, false
        };
    }else if( (tempSumPtMuMu > 0) && (tempSumPtMuMu > tempSumPtElEl) && (tempSumPtMuMu > tempSumPtElMu) && (tempSumPtMuMu > tempSumPtMuEl) ){
      m_diLepton = { 
        muons.p4[leadingSelectedMuMu.first] + muons.p4[leadingSelectedMuMu.second], 
        std::make_pair(leadingSelectedMuMu.first, leadingSelectedMuMu.second),
        std::make_pair(-1, -1),
        false, false, false, true
        };
    }else{
      m_diLepton = { myLorentzVector(), std::make_pair(-1, -1), std::make_pair(-1, -1), false, false, false, false };
    }

    diLepton_p4 = m_diLepton.p4;
    diLepton_idxs = m_diLepton.idxs;
    for(uint8_t i = 0; i<m_leptons.size(); ++i){
      if(m_leptons[i].idx == diLepton_idxs.first) diLepton_lidxs.first = i;
      if(m_leptons[i].idx == diLepton_idxs.second) diLepton_lidxs.second = i;
    }
    diLepton_isElEl = m_diLepton.isElEl;
    diLepton_isElMu = m_diLepton.isElMu;
    diLepton_isMuEl = m_diLepton.isMuEl;
    diLepton_isMuMu = m_diLepton.isMuMu;
    if(m_diLepton.lidxs.first >= 0){
      diLepton_DR = VectorUtil::DeltaR( lepton_p4[ m_diLepton.lidxs.first ], lepton_p4[ m_diLepton.lidxs.second ] );
      diLepton_DPhi = VectorUtil::DeltaPhi( lepton_p4[ m_diLepton.lidxs.first ], lepton_p4[ m_diLepton.lidxs.second ] );
      diLepton_DEta = DeltaEta( lepton_p4[ m_diLepton.lidxs.first ], lepton_p4[ m_diLepton.lidxs.second ] );
    }*/

    ///////////////////////////
    //       JETS            //
    ///////////////////////////

    const JetsProducer& jets = producers.get<JetsProducer>("jets");

    for(uint8_t ijet = 0; ijet < jets.p4.size(); ijet++){
        
      // Save the jets that pass the kinematic cuts
      if( abs(jets.p4[ijet].Eta()) < m_jetEtaCut && jets.p4[ijet].Pt() > m_jetPtCut){
        selectedJets.push_back(ijet);
          
        // Save the jets that pass the kinematic cuts and tight jetID
        if( abs(jets.p4[ijet].Eta()) < m_jetEtaCut && jets.p4[ijet].Pt() > m_jetPtCut && jets.passTightID[ijet]){
          selectedJets_tightID.push_back(ijet);
          
          bool passDRcut(true);
          for(const auto &lepton: m_leptons){
            if(VectorUtil::DeltaR(jets.p4[ijet], lepton.p4) < m_jetDRleptonCut)
              passDRcut = false;
          }
          // Save the jets that pass the kinematic cuts and tight jetID and DR(l,j)>cut using selected leptons
          if(passDRcut)
            selectedJets_tightID_DRcut.push_back(ijet);
        
        } // end selected + tightID
      } // end selected
    } // end for

    if(selectedJets_tightID_DRcut.size() >= 2){
      int jet1 = selectedJets_tightID_DRcut[0];
      int jet2 = selectedJets_tightID_DRcut[1];
      m_diJet = { 
        jets.p4[jet1] + jets.p4[jet2],
        std::make_pair(jet1, jet2)
      };
      diJet_p4 = m_diJet.p4;
      diJet_idxs = m_diJet.idxs;
      diJet_DR = VectorUtil::DeltaR(jets.p4[jet1], jets.p4[jet2]);
      diJet_DPhi = VectorUtil::DeltaPhi(jets.p4[jet1], jets.p4[jet2]);
      diJet_DEta = DeltaEta(jets.p4[jet1], jets.p4[jet2]);
    }else{
      m_diJet = { myLorentzVector(), std::make_pair(-1, -1) };
    }

    ///////////////////////////
    //       B-JETS          //
    ///////////////////////////

    // Save the selected jets that pass (or not) the different CSVv2 working points
    for(uint8_t i = 0; i < selectedJets_tightID_DRcut.size(); i++){
      uint8_t ijet = selectedJets_tightID_DRcut[i];

      float csvv2 = jets.getBTagDiscriminant(ijet, m_jetCSVv2Name);

      if(csvv2 >= m_jetCSVv2L){
        selectedBJets_CSVv2L.push_back(ijet);
        
        if(csvv2 >= m_jetCSVv2M){
          selectedBJets_CSVv2M.push_back(ijet);
          
          if(csvv2 >= m_jetCSVv2T){
            selectedBJets_CSVv2T.push_back(ijet);
          }else{
            selectedNonBJets_CSVv2T.push_back(ijet);
          } // end Tight
        
        }else{
          selectedNonBJets_CSVv2M.push_back(ijet);
        } // end Medium
      
      }else{
        selectedNonBJets_CSVv2L.push_back(ijet);
      } // end Loose
    
    } // end for

    // Choose pairs of L b-jets, from Pt- or CSVv2-ordered selected jets

    std::vector<uint8_t> selectedBJets_CSVv2L_CSVv2Ordered = selectedBJets_CSVv2L;
    std::sort(selectedBJets_CSVv2L_CSVv2Ordered.begin(), selectedBJets_CSVv2L_CSVv2Ordered.end(), jetBTagDiscriminantSorter(jets, m_jetCSVv2Name));
    if(selectedBJets_CSVv2L.size() >= 2){
      uint8_t jet1 = selectedBJets_CSVv2L[0];
      uint8_t jet2 = selectedBJets_CSVv2L[1];
      m_diBJet_PtChosen["LL"] = { jets.p4[jet1] + jets.p4[jet2], std::make_pair(jet1, jet2) };
      
      jet1 = selectedBJets_CSVv2L_CSVv2Ordered[0];
      jet2 = selectedBJets_CSVv2L_CSVv2Ordered[1];
      m_diBJet_CSVv2Chosen["LL"] = { jets.p4[jet1] + jets.p4[jet2], std::make_pair(jet1, jet2) };
    }else{
      m_diBJet_PtChosen["LL"] = { myLorentzVector(), std::make_pair(-1, -1) };
      m_diBJet_CSVv2Chosen["LL"] = { myLorentzVector(), std::make_pair(-1, -1) };
    }

    // Choose pairs of M b-jets, from Pt- or CSVv2-ordered selected jets

    std::vector<uint8_t> selectedBJets_CSVv2M_CSVv2Ordered = selectedBJets_CSVv2M;
    std::sort(selectedBJets_CSVv2M_CSVv2Ordered.begin(), selectedBJets_CSVv2M_CSVv2Ordered.end(), jetBTagDiscriminantSorter(jets, m_jetCSVv2Name));
    if(selectedBJets_CSVv2M.size() >= 2){
      uint8_t jet1 = selectedBJets_CSVv2M[0];
      uint8_t jet2 = selectedBJets_CSVv2M[1];
      m_diBJet_PtChosen["MM"] = { jets.p4[jet1] + jets.p4[jet2], std::make_pair(jet1, jet2) };
      
      jet1 = selectedBJets_CSVv2M_CSVv2Ordered[0];
      jet2 = selectedBJets_CSVv2M_CSVv2Ordered[1];
      m_diBJet_CSVv2Chosen["MM"] = { jets.p4[jet1] + jets.p4[jet2], std::make_pair(jet1, jet2) };
    }else{
      m_diBJet_PtChosen["MM"] = { myLorentzVector(), std::make_pair(-1, -1) };
      m_diBJet_CSVv2Chosen["MM"] = { myLorentzVector(), std::make_pair(-1, -1) };
    }

    // Choose pairs of T b-jets, from Pt- or CSVv2-ordered selected jets

    std::vector<uint8_t> selectedBJets_CSVv2T_CSVv2Ordered = selectedBJets_CSVv2T;
    std::sort(selectedBJets_CSVv2T_CSVv2Ordered.begin(), selectedBJets_CSVv2T_CSVv2Ordered.end(), jetBTagDiscriminantSorter(jets, m_jetCSVv2Name));
    if(selectedBJets_CSVv2T.size() >= 2){
      uint8_t jet1 = selectedBJets_CSVv2T[0];
      uint8_t jet2 = selectedBJets_CSVv2T[1];
      m_diBJet_PtChosen["TT"] = { jets.p4[jet1] + jets.p4[jet2], std::make_pair(jet1, jet2) };
      
      jet1 = selectedBJets_CSVv2T_CSVv2Ordered[0];
      jet2 = selectedBJets_CSVv2T_CSVv2Ordered[1];
      m_diBJet_CSVv2Chosen["TT"] = { jets.p4[jet1] + jets.p4[jet2], std::make_pair(jet1, jet2) };
    }else{
      m_diBJet_PtChosen["TT"] = { myLorentzVector(), std::make_pair(-1, -1) };
      m_diBJet_CSVv2Chosen["TT"] = { myLorentzVector(), std::make_pair(-1, -1) };
    }

    // Save the interesting quantitites to disk, for all diBJet working points and both ordering schemes.
    // Always take care that undefined quantities can still be accessed => it will be the categories' role
    // to determine whether a quantity is well-defined or not.

    for(const std::string &wp: {"LL", "MM", "TT"}){
      // Pt-ordered
      diBJet_p4[wp + "-Pt"] = m_diBJet_PtChosen[wp].p4;
      diBJet_idxs[wp + "-Pt"] = std::make_pair(m_diBJet_PtChosen[wp].idxs.first, m_diBJet_PtChosen[wp].idxs.second);
      if(m_diBJet_PtChosen[wp].idxs.first >=0 ){
        diBJet_DR[wp + "-Pt"] = VectorUtil::DeltaR(jets.p4[m_diBJet_PtChosen[wp].idxs.first], jets.p4[m_diBJet_PtChosen[wp].idxs.second]);
        diBJet_DPhi[wp + "-Pt"] = VectorUtil::DeltaPhi(jets.p4[m_diBJet_PtChosen[wp].idxs.first], jets.p4[m_diBJet_PtChosen[wp].idxs.second]);
        diBJet_DEta[wp + "-Pt"] = DeltaEta(jets.p4[m_diBJet_PtChosen[wp].idxs.first], jets.p4[m_diBJet_PtChosen[wp].idxs.second]);
      }else{
        diBJet_DR[wp + "-Pt"] = 0;
        diBJet_DPhi[wp + "-Pt"] = 0;
        diBJet_DEta[wp + "-Pt"] = 0;
      }
      selectedJets_diBJetExcluded[wp + "-Pt"] = std::vector<uint8_t>();
      for(uint8_t ijet = 0; ijet < selectedJets_tightID_DRcut.size(); ijet++){
        if(m_diBJet_PtChosen[wp].idxs.first != ijet && m_diBJet_PtChosen[wp].idxs.second != ijet)
          selectedJets_diBJetExcluded[wp + "-Pt"].push_back(ijet);
      }

      // CSVv2-ordered
      diBJet_p4[wp + "-CSVv2"] = m_diBJet_CSVv2Chosen[wp].p4;
      diBJet_idxs[wp + "-CSVv2"] = std::make_pair(m_diBJet_CSVv2Chosen[wp].idxs.first, m_diBJet_CSVv2Chosen[wp].idxs.second);
      if(m_diBJet_CSVv2Chosen[wp].idxs.first >=0 ){
        diBJet_DR[wp + "-CSVv2"] = VectorUtil::DeltaR(jets.p4[m_diBJet_CSVv2Chosen[wp].idxs.first], jets.p4[m_diBJet_CSVv2Chosen[wp].idxs.second]);
        diBJet_DPhi[wp + "-CSVv2"] = VectorUtil::DeltaPhi(jets.p4[m_diBJet_CSVv2Chosen[wp].idxs.first], jets.p4[m_diBJet_CSVv2Chosen[wp].idxs.second]);
        diBJet_DEta[wp + "-CSVv2"] = DeltaEta(jets.p4[m_diBJet_CSVv2Chosen[wp].idxs.first], jets.p4[m_diBJet_CSVv2Chosen[wp].idxs.second]);
      }else{
        diBJet_DR[wp + "-CSVv2"] = 0;
        diBJet_DPhi[wp + "-CSVv2"] = 0;
        diBJet_DEta[wp + "-CSVv2"] = 0;
      }
      selectedJets_diBJetExcluded[wp + "-CSVv2"] = std::vector<uint8_t>();
      for(uint8_t ijet = 0; ijet < selectedJets_tightID_DRcut.size(); ijet++){
        if(m_diBJet_CSVv2Chosen[wp].idxs.first != ijet && m_diBJet_CSVv2Chosen[wp].idxs.second != ijet)
          selectedJets_diBJetExcluded[wp + "-CSVv2"].push_back(ijet);
      }

    }

    ///////////////////////////
    //    EVENT VARIABLES    //
    ///////////////////////////
    
    // leptons-jets
   
    ll_jj_p4 = diLepton_p4 + diJet_p4;
    ll_jj_DR = VectorUtil::DeltaR(diLepton_p4, diJet_p4);
    ll_jj_DPhi = VectorUtil::DeltaPhi(diLepton_p4, diJet_p4);
    ll_jj_DEta = DeltaEta(diLepton_p4, diJet_p4);

    lj_minDR = 10;
    lj_noDRcut_minDR = 10;
    if(m_diLepton.idxs.first >= 0){
      for(uint8_t ijet = 0; ijet < selectedJets_tightID_DRcut.size(); ijet++){
        float dr = std::min(
            VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.first].p4, jets.p4[ijet]),
            VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.second].p4, jets.p4[ijet])
            );
        if(dr < lj_minDR)
          lj_minDR = dr;
      }
      for(uint8_t ijet = 0; ijet < selectedJets_tightID.size(); ijet++){
        float dr = std::min(
            VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.first].p4, jets.p4[ijet]),
            VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.second].p4, jets.p4[ijet])
            );
        if(dr < lj_noDRcut_minDR)
          lj_noDRcut_minDR = dr;
      }
    }
    
    // leptons-bjets

    for(const std::string &wp: {"LL", "MM", "TT"}){
      for(const std::string &order: {"-Pt", "-CSVv2"}){
        ll_bb_p4[wp+order] = diLepton_p4 + diBJet_p4[wp+order];
        ll_bb_DR[wp+order] = VectorUtil::DeltaR(diLepton_p4, diBJet_p4[wp+order]);
        ll_bb_DPhi[wp+order] = VectorUtil::DeltaPhi(diLepton_p4, diBJet_p4[wp+order]);
        ll_bb_DEta[wp+order] = DeltaEta(diLepton_p4, diBJet_p4[wp+order]);

        lb_minDR[wp+order] = 10;
        if(m_diLepton.idxs.first >= 0){
          float dr = std::min( {
              VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.first].p4, jets.p4[diBJet_idxs[wp+order].first]),
              VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.second].p4, jets.p4[diBJet_idxs[wp+order].first]),
              VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.first].p4, jets.p4[diBJet_idxs[wp+order].second]),
              VectorUtil::DeltaR(m_leptons[m_diLepton.lidxs.second].p4, jets.p4[diBJet_idxs[wp+order].second])
              } );
          if(dr < lb_minDR[wp+order])
            lb_minDR[wp+order] = dr;
        }
      }
    }

    // leptons-(b-)jets-MET

    const METProducer &met = producers.get<METProducer>("met");
    const METProducer &noHFmet = producers.get<METProducer>("nohf_met");

    ll_jj_pfMET_p4 = diLepton_p4 + diJet_p4 + met.p4;
    ll_pfMET_DPhi = VectorUtil::DeltaPhi(diLepton_p4, met.p4);
    jj_pfMET_DPhi = VectorUtil::DeltaPhi(diJet_p4, met.p4);
    ll_jj_pfMET_DPhi = VectorUtil::DeltaPhi(diLepton_p4 + diJet_p4, met.p4);
    
    ll_jj_noHFMET_p4 = diLepton_p4 + diJet_p4 + noHFmet.p4;
    ll_noHFMET_DPhi = VectorUtil::DeltaPhi(diLepton_p4, noHFmet.p4);
    jj_noHFMET_DPhi = VectorUtil::DeltaPhi(diJet_p4, noHFmet.p4);
    ll_jj_noHFMET_DPhi = VectorUtil::DeltaPhi(diLepton_p4 + diJet_p4, noHFmet.p4);

    for(const std::string &wp: {"LL", "MM", "TT"}){
      for(const std::string &order: {"-Pt", "-CSVv2"}){
        ll_bb_p4[wp+order] = diLepton_p4 + diBJet_p4[wp+order];
    
        ll_bb_pfMET_p4[wp+order] = diLepton_p4 + diBJet_p4[wp+order] + met.p4;
        bb_pfMET_DPhi[wp+order] = VectorUtil::DeltaPhi(diBJet_p4[wp+order], met.p4);
        ll_bb_pfMET_DPhi[wp+order] = VectorUtil::DeltaPhi(diLepton_p4 + diBJet_p4[wp+order], met.p4);
        
        ll_bb_noHFMET_p4[wp+order] = diLepton_p4 + diBJet_p4[wp+order] + noHFmet.p4;
        bb_noHFMET_DPhi[wp+order] = VectorUtil::DeltaPhi(diBJet_p4[wp+order], noHFmet.p4);
        ll_bb_noHFMET_DPhi[wp+order] = VectorUtil::DeltaPhi(diLepton_p4 + diBJet_p4[wp+order], noHFmet.p4);
      }
    }

    ///////////////////////////
    //       TRIGGER         //
    ///////////////////////////

    ///////////////////////////
    //       GEN INFO        //
    ///////////////////////////
    if (event.isRealData())
        return;

    const GenParticlesProducer& gen_particles = producers.get<GenParticlesProducer>("gen_particles");

    // 'Pruned' particles are from the hard process
    // 'Packed' particles are stable particles

    std::function<bool(size_t, size_t)> pruned_decays_from = [&pruned_decays_from, &gen_particles](size_t particle_index, size_t mother_index) -> bool {
        // Iterator over all pruned particles to find if the particle `particle_index` has `mother_index` in its decay history
        for (uint16_t index: gen_particles.pruned_mothers_index[particle_index]) {
            if (index == mother_index) {
                return true;
            }

            if (pruned_decays_from(index, mother_index))
                return true;
        }

        return false;
    };


#define ASSIGN_INDEX( X ) \
    if (flags.isLastCopy()) { \
        gen_##X = i; \
    } else { \
        gen_##X##_beforeFSR = i; \
    }

// Assign index to X if it's empty, or Y if not
#define ASSIGN_INDEX2(X, Y, ERROR) \
    if (flags.isLastCopy()) { \
        if (gen_##X == 0) \
            gen_##X = i; \
        else if (gen_##Y == 0)\
            gen_##Y = i; \
        else \
            std::cout << ERROR << std::endl; \
    } else if (flags.isFirstCopy()) { \
        if (gen_##X##_beforeFSR == 0) \
            gen_##X##_beforeFSR = i; \
        else if (gen_##Y##_beforeFSR == 0)\
            gen_##Y##_beforeFSR = i; \
        else \
            std::cout << ERROR << std::endl; \
    }

    for (size_t i = 0; i < gen_particles.pruned_pdg_id.size(); i++) {

        int16_t pdg_id = gen_particles.pruned_pdg_id[i];
        uint16_t a_pdg_id = std::abs(pdg_id);

        // We only care of particles with PDG id <= 16 (16 is neutrino tau)
        if (a_pdg_id > 16)
            continue;

        GenStatusFlags flags(gen_particles.pruned_status_flags[i]);

        if (! flags.isLastCopy() && ! flags.isFirstCopy())
            continue;

        if (! flags.fromHardProcess())
            continue;

        if (pdg_id == 6) {
            ASSIGN_INDEX(t);
            continue;
        } else if (pdg_id == -6) {
            ASSIGN_INDEX(tbar);
            continue;
        } else if (pdg_id == 5) {
            ASSIGN_INDEX(b);
            continue;
        } else if (pdg_id == -5) {
            ASSIGN_INDEX(bbar);
            continue;
        }

        if ((gen_tbar == 0) || (gen_t == 0))
            continue;

        if (gen_t != 0 && pruned_decays_from(i, gen_t)) {
            if (a_pdg_id >= 1 && a_pdg_id <= 4) {
                ASSIGN_INDEX2(jet1_t, jet2_t, "Error: more than two quarks coming from top decay");
            } else if (a_pdg_id == 11 || a_pdg_id == 13 || a_pdg_id == 15) {
                ASSIGN_INDEX(lepton_t);
            } else if (a_pdg_id == 12 || a_pdg_id == 14 || a_pdg_id == 16) {
                ASSIGN_INDEX(neutrino_t);
            } else {
                std::cout << "Error: unknown particle coming from top decay - #" << i << " ; PDG Id: " << pdg_id << std::endl;
            }
        } else if (gen_tbar != 0 && pruned_decays_from(i, gen_tbar)) {
            if (a_pdg_id >= 1 && a_pdg_id <= 4) {
                ASSIGN_INDEX2(jet1_tbar, jet2_tbar, "Error: more than two quarks coming from anti-top decay");
            } else if (a_pdg_id == 11 || a_pdg_id == 13 || a_pdg_id == 15) {
                ASSIGN_INDEX(lepton_tbar);
            } else if (a_pdg_id == 12 || a_pdg_id == 14 || a_pdg_id == 16) {
                ASSIGN_INDEX(neutrino_tbar);
            } else {
                std::cout << "Error: unknown particle coming from anti-top decay - #" << i << " ; PDG Id: " << pdg_id << std::endl;
            }
        }
    }

    if (!gen_t || !gen_tbar) {
        gen_ttbar_decay_type = NotTT;
        return;
    }

    if ((gen_jet1_t != 0) && (gen_jet2_t != 0) && (gen_jet1_tbar != 0) && (gen_jet2_tbar != 0)) {
        gen_ttbar_decay_type = Hadronic;
    } else if (
            ((gen_lepton_t != 0) && (gen_lepton_tbar == 0)) ||
            ((gen_lepton_t == 0) && (gen_lepton_tbar != 0))
            ) {

        uint16_t lepton_pdg_id;
        if (gen_lepton_t != 0)
            lepton_pdg_id = std::abs(gen_particles.pruned_pdg_id[gen_lepton_t]);
        else
            lepton_pdg_id = std::abs(gen_particles.pruned_pdg_id[gen_lepton_tbar]);

        if (lepton_pdg_id == 11)
            gen_ttbar_decay_type = Semileptonic_e;
        else if (lepton_pdg_id == 13)
            gen_ttbar_decay_type = Semileptonic_mu;
        else
            gen_ttbar_decay_type = Semileptonic_tau;
    } else {
        uint16_t lepton_t_pdg_id = std::abs(gen_particles.pruned_pdg_id[gen_lepton_t]);
        uint16_t lepton_tbar_pdg_id = std::abs(gen_particles.pruned_pdg_id[gen_lepton_tbar]);

        if (lepton_t_pdg_id == 11 && lepton_tbar_pdg_id == 11)
            gen_ttbar_decay_type = Dileptonic_ee;
        else if (lepton_t_pdg_id == 13 && lepton_tbar_pdg_id == 13)
            gen_ttbar_decay_type = Dileptonic_mumu;
        else if (lepton_t_pdg_id == 15 && lepton_tbar_pdg_id == 15)
            gen_ttbar_decay_type = Dileptonic_tautau;
        else if (
                (lepton_t_pdg_id == 11 && lepton_tbar_pdg_id == 13) ||
                (lepton_t_pdg_id == 13 && lepton_tbar_pdg_id == 11)
                ) {
            gen_ttbar_decay_type = Dileptonic_mue;
        }
        else if (
                (lepton_t_pdg_id == 11 && lepton_tbar_pdg_id == 15) ||
                (lepton_t_pdg_id == 15 && lepton_tbar_pdg_id == 11)
                ) {
            gen_ttbar_decay_type = Dileptonic_etau;
        }
        else if (
                (lepton_t_pdg_id == 13 && lepton_tbar_pdg_id == 15) ||
                (lepton_t_pdg_id == 15 && lepton_tbar_pdg_id == 13)
                ) {
            gen_ttbar_decay_type = Dileptonic_mutau;
        } else {
            std::cout << "Error: unknown dileptonic ttbar decay." << std::endl;
            gen_ttbar_decay_type = NotTT;
            return;
        }
    }

    gen_ttbar_p4 = gen_particles.pruned_p4[gen_t] + gen_particles.pruned_p4[gen_tbar];
    if (gen_t_beforeFSR != 0 && gen_tbar_beforeFSR != 0)
        gen_ttbar_beforeFSR_p4 = gen_particles.pruned_p4[gen_t_beforeFSR] + gen_particles.pruned_p4[gen_tbar_beforeFSR];
}

void TTAnalyzer::registerCategories(CategoryManager& manager, const edm::ParameterSet& config) {
  manager.new_category<TTMuMuCategory>("mumu", "Category with leading leptons as two muons", config);
  manager.new_category<TTElElCategory>("elel", "Category with leading leptons as two electrons", config);
  manager.new_category<TTMuElCategory>("muel", "Category with leading leptons as muon, electron", config);
  manager.new_category<TTElMuCategory>("elmu", "Category with leading leptons as electron, muon", config);
  
  manager.new_category<NoZTTMuMuCategory>("noZmumu", "Category with leading leptons as two muons, excluding the Z peak", config);
  manager.new_category<NoZTTElElCategory>("noZelel", "Category with leading leptons as two electrons, excluding the Z peak", config);
}

#include <FWCore/PluginManager/interface/PluginFactory.h>
DEFINE_EDM_PLUGIN(ExTreeMakerAnalyzerFactory, TTAnalyzer, "tt_analyzer");
