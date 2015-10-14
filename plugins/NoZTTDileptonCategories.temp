#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include <cp3_llbb/Framework/interface/ElectronsProducer.h>

#include <cp3_llbb/TTAnalysis/interface/TTAnalyzer.h>
#include <cp3_llbb/TTAnalysis/interface/TTDileptonCategories.h>
#include <cp3_llbb/TTAnalysis/interface/NoZTTDileptonCategories.h>

// ***** ***** *****
// Dilepton noZ Mu-Mu category
// ***** ***** *****

void NoZTTMuMuCategory::configure(const edm::ParameterSet& conf) {
  NoZTTDileptonCategory::configure(conf);
  TTMuMuCategory::configure(conf);
}

void NoZTTMuMuCategory::register_cuts(CutManager& manager) {
    TTMuMuCategory::register_cuts(manager);
    manager.new_cut("ll_mass_noZ", "116 > mll > 86");
}

void NoZTTMuMuCategory::evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    TTMuMuCategory::evaluate_cuts_post_analyzers(manager, producers, analyzers);

    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");

    int selMu1 = tt_analyzer.selectedLeadingMuMu.first;
    int selMu2 = tt_analyzer.selectedLeadingMuMu.second;

    if( selMu1 >= 0 ){
        if( (muons.p4[selMu1] + muons.p4[selMu2]).M() < m_mll_cut_low || (muons.p4[selMu1] + muons.p4[selMu2]).M() > m_mll_cut_high )
            manager.pass_cut("ll_mass_noZ");
    }
}

// ***** ***** *****
// Dilepton noZ El-El category
// ***** ***** *****

void NoZTTElElCategory::configure(const edm::ParameterSet& conf) {
  NoZTTDileptonCategory::configure(conf);
  TTElElCategory::configure(conf);
}

void NoZTTElElCategory::register_cuts(CutManager& manager) {
    TTElElCategory::register_cuts(manager);
    manager.new_cut("ll_mass_noZ", "116 > mll > 86");
}

void NoZTTElElCategory::evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {
    TTElElCategory::evaluate_cuts_post_analyzers(manager, producers, analyzers);
    
    const TTAnalyzer& tt_analyzer = analyzers.get<TTAnalyzer>("tt");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    int selEl1 = tt_analyzer.selectedLeadingElEl.first;
    int selEl2 = tt_analyzer.selectedLeadingElEl.second;

    if( selEl1 >= 0 ){
        if( (electrons.p4[selEl1] + electrons.p4[selEl2]).M() < m_mll_cut_low || (electrons.p4[selEl1] + electrons.p4[selEl2]).M() > m_mll_cut_high )
            manager.pass_cut("ll_mass_noZ");
    }
}

