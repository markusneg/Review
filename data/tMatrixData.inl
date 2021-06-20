#pragma once

#include "tMatrixData.hpp"

#include "../common/tSetting.hpp"

inline uint tMatrixData::n_rows() const {return m_matrix.n_rows;}
inline uint tMatrixData::n_cols() const {return m_matrix.n_cols;}

inline arma::Mat<double>& tMatrixData::matrix() {return m_matrix;}
inline const arma::Mat<double>& tMatrixData::matrix() const {return m_matrix;}
