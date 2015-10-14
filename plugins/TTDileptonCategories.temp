#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include <cp3_llbb/Framework/interface/ElectronsProducer.h>

#include <cp3_llbb/TTAnalysis/interface/TTDileptonCategories.h>
#include <cp3_llbb/TTAnalysis/interface/TTAnalyzer.h>

// ***** ***** *****
// Dilepton category: evaluate Z veto mass cuts
// ***** ***** *****

void TTDileptonCategory::register_cuts(CutManager& manager) {
    manager.new_cut("ll_mass_Zveto", "116 > mll > 86");
};

void TTDileptonCategory::evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    const TTAnalyzer& tt = analyzers.get<TTAnalyzer>("tt");
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    // El-El
    for(unsigned int iele1 = 0; iele1 < tt.selectedElectrons.size(); iele1++){
        for(unsigned int iele2 = iele1+1; iele2 < tt.selectedElectrons.size(); iele2++){
            if( electrons.charge[ tt.selectedElectrons[iele1] ] * electrons.charge[ tt.selectedElectrons[iele2] ] < 0 ){
                LorentzVector dilep = electrons.p4[ tt.selectedElectrons[iele1] ] + electrons.p4[ tt.selectedElectrons[iele2] ];
                if( dilep.M() > m_mll_ZVetoCut_low && dilep.M() < m_mll_ZVetoCut_high)
                    return;
            }
        }
    }

    // Mu-Mu
    for(unsigned int imu1 = 0; imu1 < tt.selectedMuons.size(); imu1++){
        for(unsigned int imu2 = imu1+1; imu2 < tt.selectedMuons.size(); imu2++){
            if( muons.charge[ tt.selectedMuons[imu1] ] * muons.charge[ tt.selectedMuons[imu2] ] < 0 ){
                LorentzVector dilep = muons.p4[ tt.selectedMuons[imu1] ] + muons.p4[ tt.selectedMuons[imu2] ];
                if( dilep.M() > m_mll_ZVetoCut_low && dilep.M() < m_mll_ZVetoCut_high)
                    return;
            }
        }
    }

    manager.pass_cut("ll_mass_Zveto");
}

// ***** ***** *****
// Dilepton Mu-Mu category
// ***** ***** *****
bool TTMuMuCategory::event_in_category_pre_analyzers(const ProducersManager& producers) const {
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    return muons.p4.size() >= 2 ;
};

bool TTMuMuCategory::event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    int selMu1 = tt_analyzer.selectedLeadingMuMu.first;
    int selMu2 = tt_analyzer.selectedLeadingMuMu.second;

    if( selMu1 < 0 )
        return false;

    if( tt_analyzer.selectedLeadingElMu.first >= 0){
        if( muons.p4[selMu1].Pt() < electrons.p4[ tt_analyzer.selectedLeadingElMu.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingElEl.first >= 0){
        if( muons.p4[selMu1].Pt() < electrons.p4[ tt_analyzer.selectedLeadingElEl.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingMuEl.second >= 0){
        if( muons.p4[selMu2].Pt() < electrons.p4[ tt_analyzer.selectedLeadingMuEl.second ].Pt() )
            return false;
    }

    return true;
};

void TTMuMuCategory::register_cuts(CutManager& manager) {
    TTDileptonCategory::register_cuts(manager);
    manager.new_cut("ll_mass", "mll > 20");
};

void TTMuMuCategory::evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    TTDileptonCategory::evaluate_cuts_post_analyzers(manager, producers, analyzers);
    
    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");

    int selMu1 = tt_analyzer.selectedLeadingMuMu.first;
    int selMu2 = tt_analyzer.selectedLeadingMuMu.second;

    if( selMu1 >= 0 ){
        if( (muons.p4[selMu1] + muons.p4[selMu2]).M() > m_mll_cut)
            manager.pass_cut("ll_mass");
    }
}

// ***** ***** *****
// Dilepton Mu-El category
// ***** ***** *****
bool TTMuElCategory::event_in_category_pre_analyzers(const ProducersManager& producers) const {
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");
    return muons.p4.size() >= 1 && electrons.p4.size() >= 1;
};

bool TTMuElCategory::event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    int selMu = tt_analyzer.selectedLeadingMuEl.first;
    int selEl = tt_analyzer.selectedLeadingMuEl.second;

    if( selMu < 0 )
        return false;

    if( tt_analyzer.selectedLeadingElMu.first >= 0){
        if( muons.p4[selMu].Pt() < electrons.p4[ tt_analyzer.selectedLeadingElMu.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingElEl.first >= 0){
        if( muons.p4[selMu].Pt() < electrons.p4[ tt_analyzer.selectedLeadingElEl.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingMuMu.second >= 0){
        if( electrons.p4[selEl].Pt() < muons.p4[ tt_analyzer.selectedLeadingMuMu.second ].Pt() )
            return false;
    }

    return true;
};

void TTMuElCategory::register_cuts(CutManager& manager) {
    TTDileptonCategory::register_cuts(manager);
};

void TTMuElCategory::evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    TTDileptonCategory::evaluate_cuts_post_analyzers(manager, producers, analyzers);
}

// ***** ***** *****
// Dilepton El-Mu category
// ***** ***** *****
bool TTElMuCategory::event_in_category_pre_analyzers(const ProducersManager& producers) const {
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");
    return muons.p4.size() >= 1 && electrons.p4.size() >= 1;
};

bool TTElMuCategory::event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    int selEl = tt_analyzer.selectedLeadingElMu.first;
    int selMu = tt_analyzer.selectedLeadingElMu.second;

    if( selEl < 0 )
        return false;

    if( tt_analyzer.selectedLeadingMuEl.first >= 0){
        if( electrons.p4[selEl].Pt() < muons.p4[ tt_analyzer.selectedLeadingMuEl.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingMuMu.first >= 0){
        if( electrons.p4[selEl].Pt() < muons.p4[ tt_analyzer.selectedLeadingMuMu.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingElEl.second >= 0){
        if( muons.p4[selMu].Pt() < electrons.p4[ tt_analyzer.selectedLeadingElEl.second ].Pt() )
            return false;
    }

    return true;
};

void TTElMuCategory::register_cuts(CutManager& manager) {
    TTDileptonCategory::register_cuts(manager);
};

void TTElMuCategory::evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    TTDileptonCategory::evaluate_cuts_post_analyzers(manager, producers, analyzers);
}

// ***** ***** *****
// Dilepton El-El category
// ***** ***** *****
bool TTElElCategory::event_in_category_pre_analyzers(const ProducersManager& producers) const {
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");
    return electrons.p4.size() >= 2 ;
};

bool TTElElCategory::event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    int selEl1 = tt_analyzer.selectedLeadingElEl.first;
    int selEl2 = tt_analyzer.selectedLeadingElEl.second;

    if( selEl1 < 0 )
        return false;

    if( tt_analyzer.selectedLeadingMuEl.first >= 0){
        if( electrons.p4[selEl1].Pt() < muons.p4[ tt_analyzer.selectedLeadingMuEl.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingMuMu.first >= 0){
        if( electrons.p4[selEl1].Pt() < muons.p4[ tt_analyzer.selectedLeadingMuMu.first ].Pt() )
            return false;
    }

    if( tt_analyzer.selectedLeadingElMu.second >= 0){
        if( electrons.p4[selEl2].Pt() < muons.p4[ tt_analyzer.selectedLeadingElMu.second ].Pt() )
            return false;
    }

    return true;
};

void TTElElCategory::register_cuts(CutManager& manager) {
    TTDileptonCategory::register_cuts(manager);
    manager.new_cut("ll_mass", "mll > 20");
};

void TTElElCategory::evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    TTDileptonCategory::evaluate_cuts_post_analyzers(manager, producers, analyzers);
    
    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    int selEl1 = tt_analyzer.selectedLeadingElEl.first;
    int selEl2 = tt_analyzer.selectedLeadingElEl.second;

    if( selEl1 >= 0 ){
        if( (electrons.p4[selEl1] + electrons.p4[selEl2]).M() > m_mll_cut)
            manager.pass_cut("ll_mass");
    }
}
