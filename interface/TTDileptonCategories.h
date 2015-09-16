#ifndef DILEPTONCATEGORIES_H
#define DILEPTONCATEGORIES_H

#include <cp3_llbb/Framework/interface/Category.h>
#include <cp3_llbb/Framework/interface/Analyzer.h>

class TTDileptonCategory: public Category {
    public:
        virtual void configure(const edm::ParameterSet& conf) override {
            m_mll_cut = conf.getUntrackedParameter<double>("mll_cut", 20);
        }

    protected:
        float m_mll_cut;
};

class TTMuMuCategory: public TTDileptonCategory {
    public:
        virtual bool event_in_category_pre_analyzers(const ProducersManager& producers) const override;
        virtual bool event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
        virtual void register_cuts(CutManager& manager) override;
        virtual void evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
};

class TTMuElCategory: public TTDileptonCategory {
    public:
        virtual bool event_in_category_pre_analyzers(const ProducersManager& producers) const override;
        virtual bool event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
};

class TTElMuCategory: public TTDileptonCategory {
    public:
        virtual bool event_in_category_pre_analyzers(const ProducersManager& producers) const override;
        virtual bool event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
};

class TTElElCategory: public TTDileptonCategory {
    public:
        virtual bool event_in_category_pre_analyzers(const ProducersManager& producers) const override;
        virtual bool event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
        virtual void register_cuts(CutManager& manager) override;
        virtual void evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const override;
};

#endif
