#pragma once

#include "tFilter.hpp"
#include "../common/tSetting.hpp"
#include "../data/tDataReference.hpp"


class tRotationFilter : public tFilter
{

public:

    T_TYPEDEFS(tRotationFilter)

    ~tRotationFilter();

    void resetRotation();

    // start or stop auto rotation. Filter will look for Observers and maximize/minimize their fitness values
    void startAutoRotation();
    void stopAutoRotation();

    /// [TBD] nice solution for whole-window-input (registerInput etc)
    //std::vector<tData*> getInputData() override;

    // swap axes so that rotation is seen as data (U) -> transfer "singular values" to it
    void swapAxes(bool swap = true);
    bool isSwapped() const;

    // returns true when given inputs form a valid rotation
    bool isValidRotation() const;

    // realign non-blocked components for max variance (i.e. SVD)
    void realignForVariance();

    // orthogonal random rotation of non-blocked components
    void randomRotation();

    void setInputWindow(tWindow* window) override;

protected:

    bool updateInputWindow(const tList<tWindow*>&) override;

    tRotationFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    // initial rotation states changed, regard them and establish correct matrix extents. By default, tries to auto-detect the case
    void inputChanged(bool dataChanged = false, bool rotationChanged = false);

    void prepareDifferentialRotation(); // buffers current state of output fields
    void pairwiseRotation(uint i1, uint i2, double alpha, bool apply = false, bool oblique = false); // if apply is given, buffer is updated columnwise
    void resetRotation(uint i0, uint i1); // selectively reset output to differential-buffered columns

    // control methods for auto rotation
    bool initAutoRotation();
    void autoRotate();      // this could be a public method. However, prepareDifferentialRotation() has to be called before externally

    //void lockInputData();
    //void unlockInputData();

    void outputChanged(bool signal = true, bool delayed = false, bool wait = false); // same as for tMatrixData with all outputs

    // rudimentary storage of rotational states
    void storeRotation();
    void restoreRotation();

    tSetting<tMatrixDataSelection> m_inputData, m_inputRotation;
    //tMatrixData::ConstRef m_rotation;

    //std::vector<tMatrixData::ConstOwnership> m_lockedInput;
    tMatrixData::Ownership m_outputData, m_outputRotation;

    // dependend on swapped-state, points on actual rotation and data (input, cache and output each)
    arma::mat *m_Uc, *m_Vc, *m_Uo, *m_Vo;
    const tMatrixDataSelection *m_Ui, *m_Vi;
    bool m_swapped = false;
    bool m_validRotation = false;

    // used to cache the inputs for rapid access and differential rotation
    arma::mat m_dataCache, m_rotationCache;

    //arma::mat m_coordinates, m_coordinatesBuffer;
    
    //std::vector<arma::mat> m_buffer;
    arma::mat m_rot = arma::mat(2,2);

    arma::mat m_sbuf, m_lbuf;
    tSetting<tDiscreteSelection> m_comps, m_observations, m_fixed, m_fixedAxis, m_fixedCoords, m_targetComps;
    tSetting<bool> m_swapAxes = false;
    tSetting<uint> m_im, m_in;
    tSetting<double> m_arResolution;
    tSetting<uint> m_arBandwidth;
    tSetting<uint> m_arResVariation = 0, m_arResVariationSteps = 100;
    QSlider* m_slRot;
    tSetting<bool> m_oblique;
    tSetting<uint> m_iom, m_ion;
    tSetting<double> m_betaMin = 3;


    std::mutex m_rotationMutex;

    friend class Review;

    T_FILTER_COPYABLE(tRotationFilter)

    struct Private;
    Private* m_private;
};


