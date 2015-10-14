#ifndef NOZTTDILEPTONCATEGORIES_H
#define NOZTTDILEPTONCATEGORIES_H

#include <cp3_llbb/TTAnalysis/interface/TTDileptonCategories.h>

class NoZTTDileptonCategory {
    public:
        virtual void configure(const edm::ParameterSet& conf) {
            m_mll_cut_low = conf.getUntrackedParameter<double>("mll_ZVetoCut_low", 86);
            m_mll_cut_high = conf.getUntrackedParameter<double>("mll_ZVetoCut_high", 116);
        }

    protected:
        float m_mll_cut_low, m_mll_cut_high;
};

class NoZTTMuMuCategory: public NoZTTDileptonCategory, public TTMuMuCategory {
    public:
        virtual void configure(const edm::ParameterSet& conf);
        virtual void register_cuts(CutManager& manager) override;
        virtual void evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
};

class NoZTTElElCategory: public NoZTTDileptonCategory, public TTElElCategory {
    public:
        virtual void configure(const edm::ParameterSet& conf);
        virtual void register_cuts(CutManager& manager) override;
        virtual void evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
};

#endif
