/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2019 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file lp_data/HSimplex.h
 * @brief 
 * @author Julian Hall, Ivet Galabova, Qi Huangfu and Michael Feldmeier
 */
#ifndef SIMPLEX_HSIMPLEX_H_
#define SIMPLEX_HSIMPLEX_H_

#include "HConfig.h"
#include "SimplexConst.h" // For simplex strategy constants
#include "HighsIO.h"
#include "HighsUtils.h"
#include "HighsModelObject.h"
#include <cassert>
#include <vector>
#include <cstring> // For strcmp

/**
 * @brief Class for simplex utilities
 */
class HSimplex {
 public:
  
  void clear_solver_lp_data(
	     HighsModelObject & highs_model_object //!< Model object in which data for LP to be solved is to be cleared
	     ) {
    HighsSimplexInfo &simplex_info_ = highs_model_object.simplex_info_;
    simplex_info_.solver_lp_has_matrix_col_wise = false;
    simplex_info_.solver_lp_has_matrix_row_wise = false;
    simplex_info_.solver_lp_has_dual_steepest_edge_weights = false;
    simplex_info_.solver_lp_has_nonbasic_dual_values = false;
    simplex_info_.solver_lp_has_basic_primal_values = false;
    simplex_info_.solver_lp_has_invert = false;
    simplex_info_.solver_lp_has_fresh_invert = false;
    simplex_info_.solver_lp_has_fresh_rebuild = false;
    simplex_info_.solver_lp_has_dual_objective_value = false;
  }
  
  void clear_solver_lp(
	     HighsModelObject & highs_model_object //!< Model object in which LP to be solved is to be cleared
	     ) {
    HighsSimplexInfo &simplex_info_ = highs_model_object.simplex_info_;
    // Once the solver LP has its own basis
    //    highs_model_object.solver_basis_.valid_ = false;
    simplex_info_.solver_lp_is_transposed = false;
    simplex_info_.solver_lp_is_scaled = false;
    simplex_info_.solver_lp_is_permuted = false;
    simplex_info_.solver_lp_is_tightened = false;
    clear_solver_lp_data(highs_model_object);
  }
  
  void options(
	       HighsModelObject & highs_model_object, //!< Model object in which simplex options are to be set
	       const HighsOptions& opt                //!< HiGHS options
	       ) {
    HighsSimplexInfo &simplex_info_ = highs_model_object.simplex_info_;
    
    // Copy values of HighsOptions for the simplex solver
    // TODO: Get this right with proper simplex strategy
    simplex_info_.simplex_strategy = opt.simplex_strategy;
    simplex_info_.crash_strategy = opt.simplex_crash_strategy;
    simplex_info_.dual_edge_weight_strategy = opt.simplex_dual_edge_weight_strategy;
    simplex_info_.price_strategy = opt.simplex_price_strategy;
    simplex_info_.primal_feasibility_tolerance = opt.primal_feasibility_tolerance;
    simplex_info_.dual_feasibility_tolerance = opt.dual_feasibility_tolerance;
    simplex_info_.dual_objective_value_upper_bound = opt.dual_objective_value_upper_bound;
    simplex_info_.perturb_costs = opt.simplex_perturb_costs;
    simplex_info_.iteration_limit = opt.simplex_iteration_limit;
    simplex_info_.update_limit = opt.simplex_update_limit;
    simplex_info_.highs_run_time_limit = opt.highs_run_time_limit;
    
    simplex_info_.transpose_solver_lp = opt.transpose_solver_lp;
    simplex_info_.scale_solver_lp = opt.scale_solver_lp;
    simplex_info_.permute_solver_lp = opt.permute_solver_lp;
    simplex_info_.tighten_solver_lp = opt.tighten_solver_lp;
    
    // Set values of internal options
    
    // Options for reporting timing
    simplex_info_.report_simplex_inner_clock = true;//false;
    simplex_info_.report_simplex_outer_clock = false;
#ifdef HiGHSDEV
    simplex_info_.report_simplex_phases_clock = true;//false;
    // Option for analysing simplex iterations
    simplex_info_.analyseLp = false;
    simplex_info_.analyseSimplexIterations = true;//false
    simplex_info_.analyseLpSolution = false;
    simplex_info_.analyse_invert_time = false;
    simplex_info_.analyseRebuildTime = false;
#endif
    
  }
  
  void update_solver_lp_status_flags(
				 HighsModelObject &highs_model_object,
				 LpAction action
				 ) {
    
    HighsSimplexInfo &simplex_info_ = highs_model_object.simplex_info_;
    switch (action) {
    case LpAction::TRANSPOSE:
#ifdef HIGHSDEV
      printf(" LpAction::TRANSPOSE\n");
#endif
      simplex_info_.solver_lp_is_transposed = true;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::SCALE:
#ifdef HIGHSDEV
      printf(" LpAction::SCALE\n");
#endif
      simplex_info_.solver_lp_is_scaled = true;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::PERMUTE:
#ifdef HIGHSDEV
      printf(" LpAction::PERMUTE\n");
#endif
      simplex_info_.solver_lp_is_permuted = true;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::TIGHTEN:
#ifdef HIGHSDEV
      printf(" LpAction::TIGHTEN\n");
#endif
      simplex_info_.solver_lp_is_tightened = true;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::NEW_COSTS:
#ifdef HIGHSDEV
      printf(" LpAction::NEW_COSTS\n");
#endif
      //      initCost();
      simplex_info_.solver_lp_has_nonbasic_dual_values = false;
      simplex_info_.solver_lp_has_fresh_rebuild = false;
      simplex_info_.solver_lp_has_dual_objective_value = false;
      break;
    case LpAction::NEW_BOUNDS:
#ifdef HIGHSDEV
      printf(" LpAction::NEW_BOUNDS\n");
#endif
      //      simplex_info_.solver_lp_ = true;
      //     initBound();
      //     initValue();
      simplex_info_.solver_lp_has_basic_primal_values = false;
      simplex_info_.solver_lp_has_fresh_rebuild = false;
      simplex_info_.solver_lp_has_dual_objective_value = false;
      break;
    case LpAction::NEW_BASIS:
#ifdef HIGHSDEV
      printf(" LpAction::NEW_BASIS\n");
#endif
      highs_model_object.basis_.valid_ = true;
      //      highs_model_object.solver_basis_.valid_ = false;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::NEW_COLS:
#ifdef HIGHSDEV
      printf(" LpAction::NEW_COLS\n");
#endif
      highs_model_object.basis_.valid_ = true;
      //      highs_model_object.solver_basis_.valid_ = false;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::NEW_ROWS:
#ifdef HIGHSDEV
      printf(" LpAction::NEW_ROWS\n");
#endif
      highs_model_object.basis_.valid_ = true;
      //      highs_model_object.solver_basis_.valid_ = false;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::DEL_COLS:
#ifdef HIGHSDEV
      printf(" LpAction::DEL_COLS\n");
#endif
      highs_model_object.basis_.valid_ = false;
      //      highs_model_object.solver_basis_.valid_ = false;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::DEL_ROWS:
#ifdef HIGHSDEV
      printf(" LpAction::DEL_ROWS\n");
#endif
      highs_model_object.basis_.valid_ = false;
      //      highs_model_object.solver_basis_.valid_ = false;
      clear_solver_lp_data(highs_model_object);
      break;
    case LpAction::DEL_ROWS_BASIS_OK:
#ifdef HIGHSDEV
      printf(" LpAction::DEL_ROWS_BASIS_OK\n");
#endif
      //      simplex_info_.solver_lp_ = true;
      break;
    default:
#ifdef HIGHSDEV
      printf(" Unrecognised LpAction::%d\n", (int) action);
#endif
      break;
    }
  }

  void report_solver_lp_status_flags(
				 HighsModelObject &highs_model_object
				 ) {
    HighsSimplexInfo &simplex_info_ = highs_model_object.simplex_info_;
    printf("\nReporting solver status and flags:\n\n");
    printf("  is_transposed =                  %d\n", simplex_info_.solver_lp_is_transposed);
    printf("  is_scaled =                      %d\n", simplex_info_.solver_lp_is_scaled);
    printf("  is_permuted =                    %d\n", simplex_info_.solver_lp_is_permuted);
    printf("  is_tightened =                   %d\n", simplex_info_.solver_lp_is_tightened);
    printf("  has_matrix_col_wise =            %d\n", simplex_info_.solver_lp_has_matrix_col_wise);
    printf("  has_matrix_row_wise =            %d\n", simplex_info_.solver_lp_has_matrix_row_wise);
    printf("  has_dual_steepest_edge_weights = %d\n", simplex_info_.solver_lp_has_dual_steepest_edge_weights);
    printf("  has_nonbasic_dual_values =       %d\n", simplex_info_.solver_lp_has_nonbasic_dual_values);
    printf("  has_basic_primal_values =        %d\n", simplex_info_.solver_lp_has_basic_primal_values);
    printf("  has_invert =                     %d\n", simplex_info_.solver_lp_has_invert);
    printf("  has_fresh_invert =               %d\n", simplex_info_.solver_lp_has_fresh_invert);
    printf("  has_fresh_rebuild =              %d\n", simplex_info_.solver_lp_has_fresh_rebuild);
    printf("  has_dual_objective_value =       %d\n", simplex_info_.solver_lp_has_dual_objective_value);
  }
  void computeDualObjectiveValue(
				 HighsModelObject &highs_model_object,
				 int phase = 2
				 ) {
    HighsLp &lp_ = highs_model_object.solver_lp_;
    HighsSimplexInfo &simplex_info_ = highs_model_object.simplex_info_;
    
    simplex_info_.dualObjectiveValue = 0;
    const int numTot = lp_.numCol_ + lp_.numRow_;
    for (int i = 0; i < numTot; i++) {
      if (highs_model_object.basis_.nonbasicFlag_[i]) {
	simplex_info_.dualObjectiveValue += simplex_info_.workValue_[i] * simplex_info_.workDual_[i];
      }
    }
    if (phase != 1) {
      simplex_info_.dualObjectiveValue *= highs_model_object.scale_.cost_;
      simplex_info_.dualObjectiveValue -= lp_.offset_;
    }
    // Now have dual objective value
    simplex_info_.solver_lp_has_dual_objective_value = true;
  }
  
  void initialiseSolverLpRandomVectors(
				       HighsModelObject &highs_model
				       ) {
    HighsSimplexInfo &simplex_info_ = highs_model.simplex_info_;
    const int numCol = highs_model.solver_lp_.numCol_;
    const int numTot = highs_model.solver_lp_.numCol_ + highs_model.solver_lp_.numRow_;
    // Instantiate and (re-)initialise the random number generator
    HighsRandom &random = highs_model.random_;
    random.initialise();
    //
    // Generate a random permutation of the column indices
    simplex_info_.numColPermutation_.resize(numCol);
    int *numColPermutation = &simplex_info_.numColPermutation_[0];
    for (int i = 0; i < numCol; i++) numColPermutation[i] = i;
    for (int i = numCol - 1; i >= 1; i--) {
      int j = random.integer() % (i + 1);
      std::swap(numColPermutation[i], numColPermutation[j]);
    }
    
    // Re-initialise the random number generator and generate the
    // random vectors in the same order as hsol to maintain repeatable
    // performance
    random.initialise();
    //
    // Generate a random permutation of all the indices
    simplex_info_.numTotPermutation_.resize(numTot);
    int *numTotPermutation = &simplex_info_.numTotPermutation_[0];
    for (int i = 0; i < numTot; i++) numTotPermutation[i] = i;
    for (int i = numTot - 1; i >= 1; i--) {
      int j = random.integer() % (i + 1);
      std::swap(numTotPermutation[i], numTotPermutation[j]);
    }
    
    // Generate a vector of random reals 
    simplex_info_.numTotRandomValue_.resize(numTot);
    double *numTotRandomValue = &simplex_info_.numTotRandomValue_[0];
    for (int i = 0; i < numTot; i++) {
      numTotRandomValue[i] = random.fraction();
    }
    
  }
  
  // TRANSPOSE:
  
  void transpose_solver_lp(HighsModelObject &highs_model) {
    HighsSimplexInfo &simplex_info_ = highs_model.simplex_info_;
#ifdef HiGHSDEV
    printf("Called transpose_solver_lp: simplex_info_.solver_lp_is_transposed = %d\n", simplex_info_.solver_lp_is_transposed);
#endif
    if (simplex_info_.solver_lp_is_transposed) return;
    HighsLp& primal_lp = highs_model.lp_;
    
    int primalNumCol = primal_lp.numCol_;
    int primalNumRow = primal_lp.numRow_;
    
    int transposeCancelled = 0;
    if (1.0 * primalNumCol / primalNumRow > 0.2) {
      //        cout << "transpose-cancelled-by-ratio" << endl;
      transposeCancelled = 1;
      return;
    }
    
    vector<int>& primalAstart = primal_lp.Astart_;
    vector<int>& primalAindex = primal_lp.Aindex_;
    vector<double>& primalAvalue = primal_lp.Avalue_;
    vector<double>& primalColCost = primal_lp.colCost_;
    vector<double>& primalColLower = primal_lp.colLower_;
    vector<double>& primalColUpper = primal_lp.colUpper_;
    vector<double>& primalRowLower = primal_lp.rowLower_;
    vector<double>& primalRowUpper = primal_lp.rowUpper_;
    
    HighsLp& dual_lp = highs_model.solver_lp_;
    /*
      vector<int>& dualAstart = dual_lp.Astart_;
      vector<int>& dualAindex = dual_lp.Aindex_;
      vector<double>& dualAvalue = dual_lp.Avalue_;
      vector<double>& dualColCost = dual_lp.colCost_;
      vector<double>& dualColLower = dual_lp.colLower_;
      vector<double>& dualColUpper = dual_lp.colUpper_;
      vector<double>& dualRowLower = dual_lp.rowLower_;
      vector<double>& dualRowUpper = dual_lp.rowUpper_;
    */
    
    // Convert primal cost to dual bound
    const double inf = HIGHS_CONST_INF;
    vector<double> dualRowLower(primalNumCol);
    vector<double> dualRowUpper(primalNumCol);
    for (int j = 0; j < primalNumCol; j++) {
      double lower = primalColLower[j];
      double upper = primalColUpper[j];
      
      /*
       * Primal      Dual
       * Free        row = c
       * x > 0       row < c
       * x < 0       row > c
       * x = 0       row free
       * other       cancel
       */
      
      if (lower == -inf && upper == inf) {
	dualRowLower[j] = primalColCost[j];
	dualRowUpper[j] = primalColCost[j];
      } else if (lower == 0 && upper == inf) {
	dualRowLower[j] = -inf;
	dualRowUpper[j] = primalColCost[j];
      } else if (lower == -inf && upper == 0) {
	dualRowLower[j] = primalColCost[j];
	dualRowUpper[j] = +inf;
      } else if (lower == 0 && upper == 0) {
	dualRowLower[j] = -inf;
	dualRowUpper[j] = +inf;
      } else {
	transposeCancelled = 1;
	break;
      }
    }
    
    // Check flag
    if (transposeCancelled == 1) {
      //        cout << "transpose-cancelled-by-column" << endl;
      return;
    }
    
    // Convert primal row bound to dual variable cost
    vector<double> dualColLower(primalNumRow);
    vector<double> dualColUpper(primalNumRow);
    vector<double> dualCost(primalNumRow);
    for (int i = 0; i < primalNumRow; i++) {
      double lower = primalRowLower[i];
      double upper = primalRowUpper[i];
      
      /*
       * Primal      Dual
       * row = b     Free
       * row < b     y < 0
       * row > b     y > 0
       * row free    y = 0
       * other       cancel
       */
      
      if (lower == upper) {
	dualColLower[i] = -inf;
	dualColUpper[i] = +inf;
	dualCost[i] = -lower;
      } else if (lower == -inf && upper != inf) {
	dualColLower[i] = -inf;
	dualColUpper[i] = 0;
	dualCost[i] = -upper;
      } else if (lower != -inf && upper == inf) {
	dualColLower[i] = 0;
	dualColUpper[i] = +inf;
	dualCost[i] = -lower;
      } else if (lower == -inf && upper == inf) {
	dualColLower[i] = 0;
	dualColUpper[i] = 0;
	dualCost[i] = 0;
      } else {
	transposeCancelled = 1;
	break;
      }
    }
    
    // Check flag
    if (transposeCancelled == 1) {
      //        cout << "transpose-cancelled-by-row" << endl;
      return;
    }
    
    // We can now really transpose things
    vector<int> iwork(primalNumRow, 0);
    vector<int> ARstart(primalNumRow + 1, 0);
    int AcountX = primalAindex.size();
    vector<int> ARindex(AcountX);
    vector<double> ARvalue(AcountX);
    for (int k = 0; k < AcountX; k++) iwork[primalAindex[k]]++;
    for (int i = 1; i <= primalNumRow; i++) ARstart[i] = ARstart[i - 1] + iwork[i - 1];
    for (int i = 0; i < primalNumRow; i++) iwork[i] = ARstart[i];
    for (int iCol = 0; iCol < primalNumCol; iCol++) {
      for (int k = primalAstart[iCol]; k < primalAstart[iCol + 1]; k++) {
	int iRow = primalAindex[k];
	int iPut = iwork[iRow]++;
	ARindex[iPut] = iCol;
	ARvalue[iPut] = primalAvalue[k];
      }
    }
    
    // Transpose the problem!
    std::swap(primalNumRow, primalNumCol);
    dual_lp.Astart_.swap(ARstart);
    dual_lp.Aindex_.swap(ARindex);
    dual_lp.Avalue_.swap(ARvalue);
    dual_lp.colLower_.swap(dualColLower);
    dual_lp.colUpper_.swap(dualColUpper);
    dual_lp.rowLower_.swap(dualRowLower);
    dual_lp.rowUpper_.swap(dualRowUpper);
    dual_lp.colCost_.swap(dualCost);
    //    cout << "problem-transposed" << endl;
    // Deduce the consequences of transposing the LP
    update_solver_lp_status_flags(highs_model, LpAction::TRANSPOSE);
      //    simplex_info_.solver_lp_is_transposed = true;
  }
  
  // SCALING:
  // Limits on scaling factors
  const double minAlwScale = 1 / 1024.0;
  const double maxAlwScale = 1024.0;
  const double maxAlwCostScale = maxAlwScale;
  const double minAlwColScale = minAlwScale;
  const double maxAlwColScale = maxAlwScale;
  const double minAlwRowScale = minAlwScale;
  const double maxAlwRowScale = maxAlwScale;
  
#ifdef HiGHSDEV
  // Information on large costs
  const double tlLargeCo = 1e5;
  int numLargeCo;
  vector<int> largeCostFlag;
  double largeCostScale;
#endif
  
  void scaleHighsModelInit(HighsModelObject &highs_model) {
    highs_model.scale_.col_.assign(highs_model.solver_lp_.numCol_, 1);
    highs_model.scale_.row_.assign(highs_model.solver_lp_.numRow_, 1);
    highs_model.scale_.cost_ = 1;
#ifdef HiGHSDEV
    //  largeCostScale = 1;
#endif
  }
  
  void scaleCosts(HighsModelObject &highs_model) {
    // Scale the costs by no less than minAlwCostScale
    double costScale = highs_model.scale_.cost_;
    double maxNzCost = 0;
    for (int iCol = 0; iCol < highs_model.solver_lp_.numCol_; iCol++) {
      if (highs_model.solver_lp_.colCost_[iCol]) {
	maxNzCost = max(fabs(highs_model.solver_lp_.colCost_[iCol]), maxNzCost);
      }
    }
    // Scaling the costs up effectively increases the dual tolerance to
    // which the problem is solved - so, if the max cost is small the
    // scaling factor pushes it up by a power of 2 so it's close to 1
    // Scaling the costs down effectively decreases the dual tolerance
    // to which the problem is solved - so this can't be done too much
    costScale = 1;
    const double ln2 = log(2.0);
    // Scale the costs if the max cost is positive and outside the range [1/16,
    // 16]
    if ((maxNzCost > 0) && ((maxNzCost < (1.0 / 16)) || (maxNzCost > 16))) {
      costScale = maxNzCost;
      costScale = pow(2.0, floor(log(costScale) / ln2 + 0.5));
      costScale = min(costScale, maxAlwCostScale);
    }
#ifdef HiGHSDEV
    HighsPrintMessage(ML_MINIMAL, "MaxNzCost = %11.4g: scaling all costs by %11.4g\ngrep_CostScale,%g,%g\n",
		      maxNzCost, costScale, maxNzCost, costScale);
#endif
    if (costScale == 1) return;
    // Scale the costs (and record of maxNzCost) by costScale, being at most
    // maxAlwCostScale
    for (int iCol = 0; iCol < highs_model.solver_lp_.numCol_; iCol++) {
      highs_model.solver_lp_.colCost_[iCol] /= costScale;
    }
    maxNzCost /= costScale;
    
#ifdef HiGHSDEV
    bool alwLargeCostScaling = false;
    /*
      if (alwLargeCostScaling && (numLargeCo > 0)) {
      // Scale any large costs by largeCostScale, being at most (a further)
      // maxAlwCostScale
      largeCostScale = maxNzCost;
      largeCostScale = pow(2.0, floor(log(largeCostScale) / ln2 + 0.5));
      largeCostScale = min(largeCostScale, maxAlwCostScale);
      printf(
      "   Scaling all |cost| > %11.4g by %11.4g\ngrep_LargeCostScale,%g,%g\n",
      tlLargeCo, largeCostScale, tlLargeCo, largeCostScale);
      for (int iCol = 0; iCol < highs_model.solver_lp_.numCol_; iCol++) {
      if (largeCostFlag[iCol]) {
      highs_model.solver_lp_.colCost_[iCol] /= largeCostScale;
      }
      }
      }
    */
    HighsPrintMessage(ML_MINIMAL, "After cost scaling\n");
    //  utils.util_analyseVectorValues("Column costs", highs_model.solver_lp_.numCol_, highs_model.solver_lp_.colCost_, false);
#endif
  }
  
  void scale_solver_lp(HighsModelObject &highs_model) {
    HighsSimplexInfo &simplex_info_ = highs_model.simplex_info_;
#ifdef HiGHSDEV
    printf("Called scale_solver_lp: simplex_info_.solver_lp_is_scaled = %d\n", simplex_info_.solver_lp_is_scaled);
#endif
    if (simplex_info_.solver_lp_is_scaled) return;
    // Scale the LP highs_model.solver_lp_, assuming all data are in place
    // Reset all scaling to 1
    HighsSimplexInfo &simplex_info = highs_model.simplex_info_;
    HighsTimer &timer = highs_model.timer_;
    timer.start(timer.scale_clock);
    scaleHighsModelInit(highs_model);
    int numCol = highs_model.solver_lp_.numCol_;
    int numRow = highs_model.solver_lp_.numRow_;
    double *colScale = &highs_model.scale_.col_[0];
    double *rowScale = &highs_model.scale_.row_[0];
    int *Astart = &highs_model.solver_lp_.Astart_[0];
    int *Aindex = &highs_model.solver_lp_.Aindex_[0];
    double *Avalue = &highs_model.solver_lp_.Avalue_[0];
    double *colCost = &highs_model.solver_lp_.colCost_[0];
    double *colLower = &highs_model.solver_lp_.colLower_[0];
    double *colUpper = &highs_model.solver_lp_.colUpper_[0];
    double *rowLower = &highs_model.solver_lp_.rowLower_[0];
    double *rowUpper = &highs_model.solver_lp_.rowUpper_[0];
    
    // Allow a switch to/from the original scaling rules
    bool originalScaling = true;
    bool alwCostScaling = true;
    if (originalScaling) alwCostScaling = false;
    
    
    // Find out range of matrix values and skip matrix scaling if all
    // |values| are in [0.2, 5]
    const double inf = HIGHS_CONST_INF;
    double min0 = inf, max0 = 0;
    for (int k = 0, AnX = Astart[numCol]; k < AnX; k++) {
      double value = fabs(Avalue[k]);
      min0 = min(min0, value);
      max0 = max(max0, value);
    }
    bool noScaling = min0 >= 0.2 && max0 <= 5;
    //   noScaling = false; printf("!!!! FORCE SCALING !!!!\n");
    if (noScaling) {
      // No matrix scaling, but possible cost scaling
#ifdef HiGHSDEV
      HighsPrintMessage(ML_MINIMAL, "grep_Scaling,%s,Obj,0,Row,1,1,Col,1,1,0\n", highs_model.lp_.model_name_.c_str());
#endif
      // Possibly scale the costs
      if (!originalScaling && alwCostScaling) scaleCosts(highs_model);
      timer.stop(timer.scale_clock);
      update_solver_lp_status_flags(highs_model, LpAction::SCALE);
      return;
    }
    // See if we want to include cost include if minimum nonzero cost is less than
    // 0.1
    double minNzCost = inf;
    for (int i = 0; i < numCol; i++) {
      if (colCost[i]) minNzCost = min(fabs(colCost[i]), minNzCost);
    }
    bool includeCost = false;
    //  if (originalScaling)
    includeCost = minNzCost < 0.1;
    
    // Search up to 6 times
    vector<double> rowMin(numRow, inf);
    vector<double> rowMax(numRow, 1 / inf);
    for (int search_count = 0; search_count < 6; search_count++) {
      // Find column scale, prepare row data
      for (int iCol = 0; iCol < numCol; iCol++) {
	// For column scale (find)
	double colMin = inf;
	double colMax = 1 / inf;
	double myCost = fabs(colCost[iCol]);
	if (includeCost && myCost != 0)
	  colMin = min(colMin, myCost), colMax = max(colMax, myCost);
	for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
	  double value = fabs(Avalue[k]) * rowScale[Aindex[k]];
	  colMin = min(colMin, value), colMax = max(colMax, value);
	}
	colScale[iCol] = 1 / sqrt(colMin * colMax);
	if (!originalScaling) {
	  // Ensure that column scale factor is not excessively large or small
	  colScale[iCol] =
            min(max(minAlwColScale, colScale[iCol]), maxAlwColScale);
	}
	// For row scale (only collect)
	for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
	  int iRow = Aindex[k];
	  double value = fabs(Avalue[k]) * colScale[iCol];
	  rowMin[iRow] = min(rowMin[iRow], value);
	  rowMax[iRow] = max(rowMax[iRow], value);
	}
      }
      
      // For row scale (find)
      for (int iRow = 0; iRow < numRow; iRow++) {
	rowScale[iRow] = 1 / sqrt(rowMin[iRow] * rowMax[iRow]);
	if (!originalScaling) {
	  // Ensure that row scale factor is not excessively large or small
	  rowScale[iRow] = min(max(minAlwRowScale, rowScale[iRow]), maxAlwRowScale);
	}
      }
      rowMin.assign(numRow, inf);
      rowMax.assign(numRow, 1 / inf);
    }
    
    // Make it numerical better
    // Also determine the max and min row and column scaling factors
    double minColScale = inf;
    double maxColScale = 1 / inf;
    double minRowScale = inf;
    double maxRowScale = 1 / inf;
    const double ln2 = log(2.0);
    for (int iCol = 0; iCol < numCol; iCol++) {
      colScale[iCol] = pow(2.0, floor(log(colScale[iCol]) / ln2 + 0.5));
      minColScale = min(colScale[iCol], minColScale);
      maxColScale = max(colScale[iCol], maxColScale);
    }
    for (int iRow = 0; iRow < numRow; iRow++) {
      rowScale[iRow] = pow(2.0, floor(log(rowScale[iRow]) / ln2 + 0.5));
      minRowScale = min(rowScale[iRow], minRowScale);
      maxRowScale = max(rowScale[iRow], maxRowScale);
    }
#ifdef HiGHSDEV
    bool excessScaling =
      (minColScale < minAlwColScale) || (maxColScale > maxAlwColScale) ||
      (minRowScale < minAlwRowScale) || (maxRowScale > maxAlwRowScale);
    
    HighsPrintMessage(ML_MINIMAL, "grep_Scaling,%s,%d,%d,Obj,%g,%d,Row,%g,%g,Col,%g,%g,%d\n",
		      highs_model.lp_.model_name_.c_str(), originalScaling, alwCostScaling, minNzCost,
		      includeCost, minColScale, maxColScale, minRowScale, maxRowScale,
		      excessScaling);
#endif
    
    // Apply scaling to matrix and bounds
    for (int iCol = 0; iCol < numCol; iCol++)
      for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++)
	Avalue[k] *= (colScale[iCol] * rowScale[Aindex[k]]);
    
    for (int iCol = 0; iCol < numCol; iCol++) {
      colLower[iCol] /= colLower[iCol] == -inf ? 1 : colScale[iCol];
      colUpper[iCol] /= colUpper[iCol] == +inf ? 1 : colScale[iCol];
      colCost[iCol] *= colScale[iCol];
    }
    for (int iRow = 0; iRow < numRow; iRow++) {
      rowLower[iRow] *= rowLower[iRow] == -inf ? 1 : rowScale[iRow];
      rowUpper[iRow] *= rowUpper[iRow] == +inf ? 1 : rowScale[iRow];
    }
    // Deduce the consequences of scaling the LP
    update_solver_lp_status_flags(highs_model, LpAction::SCALE);
#ifdef HiGHSDEV
    // Analyse the scaled LP
    //  if (simplex_info.analyse_lp) {
    //    util_analyseLp(highs_model.solver_lp_, "Scaled");
    //  }
    //  if (simplex_info_.solver_lp_is_scaled) {
    //  utils.util_analyseVectorValues("Column scaling factors", numCol, colScale, false);
    //  utils.util_analyseVectorValues("Row scaling factors", numRow, rowScale, false);
    //  }
#endif
    // Possibly scale the costs
    if (!originalScaling && alwCostScaling) scaleCosts(highs_model);
    timer.stop(timer.scale_clock);
  }
  
  // PERMUTE:
  
  void permute_solver_lp(HighsModelObject &highs_model) {
    HighsSimplexInfo &simplex_info_ = highs_model.simplex_info_;
#ifdef HiGHSDEV
    printf("Called permute_solver_lp: simplex_info_.solver_lp_is_permuted = %d\n", simplex_info_.solver_lp_is_permuted);
#endif
    if (simplex_info_.solver_lp_is_permuted) return;
    //  HighsSimplexInfo &simplex_info = highs_model.simplex_info_;
    HSimplex simplex_method_;
    simplex_method_.initialiseSolverLpRandomVectors(highs_model);
    
    int numCol = highs_model.solver_lp_.numCol_;
    vector<int>& numColPermutation = highs_model.simplex_info_.numColPermutation_;
    vector<int>& Astart = highs_model.solver_lp_.Astart_;
    vector<int>& Aindex = highs_model.solver_lp_.Aindex_;
    vector<double>& Avalue = highs_model.solver_lp_.Avalue_;
    vector<double>& colCost = highs_model.solver_lp_.colCost_;
    vector<double>& colLower = highs_model.solver_lp_.colLower_;
    vector<double>& colUpper = highs_model.solver_lp_.colUpper_;
    vector<double>& colScale = highs_model.scale_.col_;
    
    // 2. Duplicate the original data to copy from
    vector<int> saveAstart = highs_model.solver_lp_.Astart_;
    vector<int> saveAindex = highs_model.solver_lp_.Aindex_;
    vector<double> saveAvalue = highs_model.solver_lp_.Avalue_;
    vector<double> saveColCost = highs_model.solver_lp_.colCost_;
    vector<double> saveColLower = highs_model.solver_lp_.colLower_;
    vector<double> saveColUpper = highs_model.solver_lp_.colUpper_;
    vector<double> saveColScale = highs_model.scale_.col_;
    
    // 3. Generate the permuted matrix and corresponding vectors of column data
    int countX = 0;
    for (int i = 0; i < numCol; i++) {
      int fromCol = numColPermutation[i];
      Astart[i] = countX;
      for (int k = saveAstart[fromCol]; k < saveAstart[fromCol + 1]; k++) {
	Aindex[countX] = saveAindex[k];
	Avalue[countX] = saveAvalue[k];
	countX++;
      }
      colCost[i] = saveColCost[fromCol];
      colLower[i] = saveColLower[fromCol];
      colUpper[i] = saveColUpper[fromCol];
      colScale[i] = saveColScale[fromCol];
    }
    assert(Astart[numCol] == countX);
    // Deduce the consequences of permuting the LP
    update_solver_lp_status_flags(highs_model, LpAction::PERMUTE); 
  }
  
  // TIGHTEN:
  
  void tighten_solver_lp(HighsModelObject &highs_model) {
    HighsSimplexInfo &simplex_info_ = highs_model.simplex_info_;
#ifdef HiGHSDEV
    printf("Called tighten_solver_lp: simplex_info_.solver_lp_is_tightened = %d\n", simplex_info_.solver_lp_is_tightened);
#endif
    if (simplex_info_.solver_lp_is_tightened) return;
    HighsSimplexInfo &simplex_info = highs_model.simplex_info_;
    
    int numCol = highs_model.solver_lp_.numCol_;
    int numRow = highs_model.solver_lp_.numRow_;
    vector<int>& Astart = highs_model.solver_lp_.Astart_;
    vector<int>& Aindex = highs_model.solver_lp_.Aindex_;
    vector<double>& Avalue = highs_model.solver_lp_.Avalue_;
    vector<double>& colCost = highs_model.solver_lp_.colCost_;
    vector<double>& colLower = highs_model.solver_lp_.colLower_;
    vector<double>& colUpper = highs_model.solver_lp_.colUpper_;
    vector<double>& rowLower = highs_model.solver_lp_.rowLower_;
    vector<double>& rowUpper = highs_model.solver_lp_.rowUpper_;
    
    
    vector<int> iwork(numRow, 0);
    vector<int> ARstart(numRow + 1, 0);
    int AcountX = Aindex.size();
    vector<int> ARindex(AcountX);
    vector<double> ARvalue(AcountX);
    for (int k = 0; k < AcountX; k++) iwork[Aindex[k]]++;
    for (int i = 1; i <= numRow; i++) ARstart[i] = ARstart[i - 1] + iwork[i - 1];
    for (int i = 0; i < numRow; i++) iwork[i] = ARstart[i];
    for (int iCol = 0; iCol < numCol; iCol++) {
      for (int k = Astart[iCol]; k < Astart[iCol + 1]; k++) {
	int iRow = Aindex[k];
	int iPut = iwork[iRow]++;
	ARindex[iPut] = iCol;
	ARvalue[iPut] = Avalue[k];
      }
    }
    
    // Save column bounds
    vector<double> colLower_0 = highs_model.solver_lp_.colLower_;
    vector<double> colUpper_0 = highs_model.solver_lp_.colUpper_;
    
    double big_B = 1e10;
    int iPass = 0;
    for (;;) {
      int numberChanged = 0;
      for (int iRow = 0; iRow < numRow; iRow++) {
	// SKIP free rows
	if (rowLower[iRow] < -big_B && rowUpper[iRow] > big_B) continue;
	
	// possible row
	int ninfU = 0;
	int ninfL = 0;
	double xmaxU = 0.0;
	double xminL = 0.0;
	int myStart = ARstart[iRow];
	int myEnd = ARstart[iRow + 1];
	// Compute possible lower and upper ranges
	
	for (int k = myStart; k < myEnd; ++k) {
	  int iCol = ARindex[k];
	  double value = ARvalue[k];
	  double upper = value > 0 ? colUpper[iCol] : -colLower[iCol];
	  double lower = value > 0 ? colLower[iCol] : -colUpper[iCol];
	  value = fabs(value);
	  if (upper < big_B)
	    xmaxU += upper * value;
	  else
	    ++ninfU;
	  if (lower > -big_B)
	    xminL += lower * value;
	  else
	    ++ninfL;
	}
	
	// Build in a margin of error
	xmaxU += 1.0e-8 * fabs(xmaxU);
	xminL -= 1.0e-8 * fabs(xminL);
	
	double xminLmargin = (fabs(xminL) > 1.0e8) ? 1e-12 * fabs(xminL) : 0;
	double xmaxUmargin = (fabs(xmaxU) > 1.0e8) ? 1e-12 * fabs(xmaxU) : 0;
	
	// Skip redundant row : also need to consider U < L  case
	double comp_U = xmaxU + ninfU * 1.0e31;
	double comp_L = xminL - ninfL * 1.0e31;
	if (comp_U <= rowUpper[iRow] + 1e-7 && comp_L >= rowLower[iRow] - 1e-7)
	  continue;
	
	double row_L = rowLower[iRow];
	double row_U = rowUpper[iRow];
	
	// Now see if we can tighten column bounds
	for (int k = myStart; k < myEnd; ++k) {
	  double value = ARvalue[k];
	  int iCol = ARindex[k];
	  double col_L = colLower[iCol];
	  double col_U = colUpper[iCol];
	  double new_L = -HIGHS_CONST_INF;
	  double new_U = +HIGHS_CONST_INF;
	  
	  if (value > 0.0) {
	    if (row_L > -big_B && ninfU <= 1 && (ninfU == 0 || col_U > +big_B))
	      new_L = (row_L - xmaxU) / value + (1 - ninfU) * col_U - xmaxUmargin;
	    if (row_U < +big_B && ninfL <= 1 && (ninfL == 0 || col_L < -big_B))
	      new_U = (row_U - xminL) / value + (1 - ninfL) * col_L + xminLmargin;
	  } else {
	    if (row_L > -big_B && ninfU <= 1 && (ninfU == 0 || col_L < -big_B))
	      new_U = (row_L - xmaxU) / value + (1 - ninfU) * col_L + xmaxUmargin;
	    if (row_U < +big_B && ninfL <= 1 && (ninfL == 0 || col_U > +big_B))
	      new_L = (row_U - xminL) / value + (1 - ninfL) * col_U - xminLmargin;
	  }
	  
	  if (new_U < col_U - 1.0e-12 && new_U < big_B) {
	    colUpper[iCol] = max(new_U, col_L);
	    numberChanged++;
	  }
	  if (new_L > col_L + 1.0e-12 && new_L > -big_B) {
	    colLower[iCol] = min(new_L, col_U);
	    numberChanged++;
	  }
	}
      }
      
      if (numberChanged == 0) break;
      iPass++;
      if (iPass > 10) break;
    }
    
    double useTolerance = 1.0e-3;
    for (int iCol = 0; iCol < numCol; iCol++) {
      if (colUpper_0[iCol] > colLower_0[iCol] + useTolerance) {
	const double relax = 100.0 * useTolerance;
	if (colUpper[iCol] - colLower[iCol] < useTolerance + 1.0e-8) {
	  colLower[iCol] = max(colLower_0[iCol], colLower[iCol] - relax);
	  colUpper[iCol] = min(colUpper_0[iCol], colUpper[iCol] + relax);
	} else {
	  if (colUpper[iCol] < colUpper_0[iCol]) {
	    colUpper[iCol] = min(colUpper[iCol] + relax, colUpper_0[iCol]);
	  }
	  if (colLower[iCol] > colLower_0[iCol]) {
	    colLower[iCol] = min(colLower[iCol] - relax, colLower_0[iCol]);
	  }
	}
      }
    }
    simplex_info_.solver_lp_is_tightened = true;
  }

  void init_value_from_nonbasic(HighsModelObject &highs_model_object, int firstvar, int lastvar) {
    // Initialise workValue and nonbasicMove from nonbasicFlag and
    // bounds, except for boxed variables when nonbasicMove is used to
    // set workValue=workLower/workUpper
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsBasis &basis = highs_model_object.basis_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    assert(firstvar >= 0);
    const int numTot = solver_lp.numCol_ + solver_lp.numRow_;
    assert(lastvar < numTot);
    // double dl_pr_act, norm_dl_pr_act;
    // norm_dl_pr_act = 0.0;
    for (int var = firstvar; var <= lastvar; var++) {
      if (basis.nonbasicFlag_[var]) {
	// Nonbasic variable
	// double prev_pr_act = simplex_info.workValue_[var];
	if (simplex_info.workLower_[var] == simplex_info.workUpper_[var]) {
	  // Fixed
	  simplex_info.workValue_[var] = simplex_info.workLower_[var];
	  basis.nonbasicMove_[var] = NONBASIC_MOVE_ZE;
	} else if (!highs_isInfinity(-simplex_info.workLower_[var])) {
	  // Finite lower bound so boxed or lower
	  if (!highs_isInfinity(simplex_info.workUpper_[var])) {
	    // Finite upper bound so boxed
	    if (basis.nonbasicMove_[var] == NONBASIC_MOVE_UP) {
	      // Set at lower
	      simplex_info.workValue_[var] = simplex_info.workLower_[var];
	    } else if (basis.nonbasicMove_[var] == NONBASIC_MOVE_DN) {
	      // Set at upper
	      simplex_info.workValue_[var] = simplex_info.workUpper_[var];
	    } else {
	      // Invalid nonbasicMove: correct and set value at lower
	      basis.nonbasicMove_[var] = NONBASIC_MOVE_UP;
	      simplex_info.workValue_[var] = simplex_info.workLower_[var];
	    }
	  } else {
	    // Lower
	    simplex_info.workValue_[var] = simplex_info.workLower_[var];
	    basis.nonbasicMove_[var] = NONBASIC_MOVE_UP;
	  }
	} else if (!highs_isInfinity(simplex_info.workUpper_[var])) {
	  // Upper
	  simplex_info.workValue_[var] = simplex_info.workUpper_[var];
	  basis.nonbasicMove_[var] = NONBASIC_MOVE_DN;
	} else {
	  // FREE
	  simplex_info.workValue_[var] = 0;
	  basis.nonbasicMove_[var] = NONBASIC_MOVE_ZE;
	}
	// dl_pr_act = simplex_info.workValue_[var] - prev_pr_act;
	// norm_dl_pr_act += dl_pr_act*dl_pr_act;
	//      if (abs(dl_pr_act) > 1e-4) printf("Var %5d: [LB; Pr; UB] of [%8g;
	//      %8g; %8g] Du = %8g; DlPr = %8g\n",
	//					var, simplex_info.workLower_[var],
	// simplex_info.workValue_[var], simplex_info.workUpper_[var], simplex_info.workDual_[var], dl_pr_act);
      } else {
	// Basic variable
	basis.nonbasicMove_[var] = NONBASIC_MOVE_ZE;
      }
    }
    //  norm_dl_pr_act = sqrt(norm_dl_pr_act);
    //  printf("initValueFromNonbasic: ||Change in nonbasic variables||_2 is
    //  %g\n", norm_dl_pr_act);
  }

  void init_value(HighsModelObject &highs_model_object) {
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    const int numTot = solver_lp.numCol_ + solver_lp.numRow_;
    init_value_from_nonbasic(highs_model_object, 0, numTot - 1);
  }

  void init_phase2_col_bound(HighsModelObject &highs_model_object, int firstcol, int lastcol) {
    // Copy bounds and compute ranges
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    assert(firstcol >= 0);
    assert(lastcol < solver_lp.numCol_);
    for (int col = firstcol; col <= lastcol; col++) {
      simplex_info.workLower_[col] = solver_lp.colLower_[col];
      simplex_info.workUpper_[col] = solver_lp.colUpper_[col];
      simplex_info.workRange_[col] = simplex_info.workUpper_[col] - simplex_info.workLower_[col];
    }
  }

  void init_phase2_row_bound(HighsModelObject &highs_model_object, int firstrow, int lastrow) {
    // Copy bounds and compute ranges
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    assert(firstrow >= 0);
    assert(lastrow < solver_lp.numRow_);
    for (int row = firstrow; row <= lastrow; row++) {
      int var = solver_lp.numCol_ + row;
      simplex_info.workLower_[var] = -solver_lp.rowUpper_[row];
      simplex_info.workUpper_[var] = -solver_lp.rowLower_[row];
      simplex_info.workRange_[var] = simplex_info.workUpper_[var] - simplex_info.workLower_[var];
    }
  }

  void init_bound(HighsModelObject &highs_model_object, int phase = 2) {
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    // Initialise the Phase 2 bounds (and ranges). NB Phase 2 bounds
    // necessary to compute Phase 1 bounds
    init_phase2_col_bound(highs_model_object, 0, solver_lp.numCol_ - 1);
    init_phase2_row_bound(highs_model_object, 0, solver_lp.numRow_ - 1);
    if (phase == 2) return;

    // In Phase 1: change to dual phase 1 bound
    const double inf = HIGHS_CONST_INF;
    const int numTot = solver_lp.numCol_ + solver_lp.numRow_;
    for (int i = 0; i < numTot; i++) {
      if (simplex_info.workLower_[i] == -inf && simplex_info.workUpper_[i] == inf) {
	// Won't change for row variables: they should never become
	// non basic
	if (i >= solver_lp.numCol_) continue;
	simplex_info.workLower_[i] = -1000, simplex_info.workUpper_[i] = 1000;  // FREE
      } else if (simplex_info.workLower_[i] == -inf) {
	simplex_info.workLower_[i] = -1, simplex_info.workUpper_[i] = 0;  // UPPER
      } else if (simplex_info.workUpper_[i] == inf) {
	simplex_info.workLower_[i] = 0, simplex_info.workUpper_[i] = 1;  // LOWER
      } else {
	simplex_info.workLower_[i] = 0, simplex_info.workUpper_[i] = 0;  // BOXED or FIXED
      }
      simplex_info.workRange_[i] = simplex_info.workUpper_[i] - simplex_info.workLower_[i];
    }
  }

  void init_phase2_col_cost(HighsModelObject &highs_model_object, int firstcol, int lastcol) {
    // Copy the Phase 2 cost and zero the shift
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    for (int col = firstcol; col <= lastcol; col++) {
      int var = col;
      simplex_info.workCost_[var] = solver_lp.sense_ * solver_lp.colCost_[col];
      simplex_info.workShift_[var] = 0.;
    }
  }
  
  void init_phase2_row_cost(HighsModelObject &highs_model_object, int firstrow, int lastrow) {
    // Zero the cost and shift
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    for (int row = firstrow; row <= lastrow; row++) {
      int var = solver_lp.numCol_ + row;
      simplex_info.workCost_[var] = 0;
      simplex_info.workShift_[var] = 0.;
    }
  }

  void init_cost(HighsModelObject &highs_model_object, int perturb = 0) {
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsBasis &basis = highs_model_object.basis_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    // Copy the cost
    init_phase2_col_cost(highs_model_object, 0, solver_lp.numCol_ - 1);
    init_phase2_row_cost(highs_model_object, 0, solver_lp.numRow_ - 1);
    // See if we want to skip perturbation
    simplex_info.costs_perturbed = 0;
    if (perturb == 0 || simplex_info.perturb_costs == 0) return;
    simplex_info.costs_perturbed = 1;

    // Perturb the original costs, scale down if is too big
    double bigc = 0;
    for (int i = 0; i < solver_lp.numCol_; i++) bigc = max(bigc, fabs(simplex_info.workCost_[i]));
    if (bigc > 100) bigc = sqrt(sqrt(bigc));

    // If there's few boxed variables, we will just use Simple perturbation
    double boxedRate = 0;
    const int numTot = solver_lp.numCol_ + solver_lp.numRow_;
    for (int i = 0; i < numTot; i++) boxedRate += (simplex_info.workRange_[i] < 1e30);
    boxedRate /= numTot;
    if (boxedRate < 0.01) bigc = min(bigc, 1.0);
    if (bigc < 1) {
      //        bigc = sqrt(bigc);
    }

    // Determine the perturbation base
    double base = 5e-7 * bigc;

    // Now do the perturbation
    for (int i = 0; i < solver_lp.numCol_; i++) {
      double lower = solver_lp.colLower_[i];
      double upper = solver_lp.colUpper_[i];
      double xpert = (fabs(simplex_info.workCost_[i]) + 1) * base * (1 + simplex_info.numTotRandomValue_[i]);
      if (lower == -HIGHS_CONST_INF && upper == HIGHS_CONST_INF) {
	// Free - no perturb
      } else if (upper == HIGHS_CONST_INF) {  // Lower
	simplex_info.workCost_[i] += xpert;
      } else if (lower == -HIGHS_CONST_INF) {  // Upper
	simplex_info.workCost_[i] += -xpert;
      } else if (lower != upper) {  // Boxed
	simplex_info.workCost_[i] += (simplex_info.workCost_[i] >= 0) ? xpert : -xpert;
      } else {
	// Fixed - no perturb
      }
    }
    
    for (int i = solver_lp.numCol_; i < numTot; i++) {
      simplex_info.workCost_[i] += (0.5 - simplex_info.numTotRandomValue_[i]) * 1e-12;
    }
  }

  void populate_work_arrays(HighsModelObject &highs_model_object) {
    // Initialize the values
    init_cost(highs_model_object);
    init_bound(highs_model_object);
    init_value(highs_model_object);
  }

  void replace_with_logical_basis(HighsModelObject &highs_model_object) {
    // Replace basis with a logical basis then populate (where possible)
    // work* arrays
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    HighsBasis &basis = highs_model_object.basis_;
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    for (int row = 0; row < solver_lp.numRow_; row++) {
      int var = solver_lp.numCol_ + row;
      basis.nonbasicFlag_[var] = NONBASIC_FLAG_FALSE;
      basis.basicIndex_[row] = var;
    }
    for (int col = 0; col < solver_lp.numCol_; col++) {
      basis.nonbasicFlag_[col] = NONBASIC_FLAG_TRUE;
    }
    simplex_info.num_basic_logicals = solver_lp.numRow_;
    
    populate_work_arrays(highs_model_object);

    // Deduce the consequences of a new basis
    update_solver_lp_status_flags(highs_model_object, LpAction::NEW_BASIS);
   
  }

  void setup_for_solve(HighsModelObject &highs_model_object) {
    HighsLp &solver_lp = highs_model_object.solver_lp_;
    int solver_num_row = solver_lp.numRow_;
    int solver_num_col = solver_lp.numCol_;
    if (solver_num_row == 0) return;

    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    HighsBasis &basis = highs_model_object.basis_;
    HMatrix &matrix = highs_model_object.matrix_;
    HFactor &factor = highs_model_object.factor_;
    report_solver_lp_status_flags(highs_model_object);
    bool basis_valid = highs_model_object.basis_.valid_;
    printf("In setup_for_solve: basis_valid = %d \n", basis_valid);
    if (basis_valid) {
    // Model has a basis so just count the number of basic logicals
      printf("Needs to call new version of setup_num_basic_logicals(highs_mode_object);\n");
    } else {
      // Model has no basis: set up a logical basis then populate (where
      // possible) work* arrays
      replace_with_logical_basis(highs_model_object);
      printf("Called replaceWithLogicalBasis\n");
  }

  if (!(simplex_info.solver_lp_has_matrix_col_wise && simplex_info.solver_lp_has_matrix_row_wise)) {
    // Make a copy of col-wise matrix for HMatrix and create its row-wise matrix
    if (simplex_info.num_basic_logicals == solver_num_row) {
      matrix.setup_lgBs(solver_num_col, solver_num_row,
			  &solver_lp.Astart_[0],
			  &solver_lp.Aindex_[0],
			  &solver_lp.Avalue_[0]);
      //      printf("Called matrix_->setup_lgBs\n");cout<<flush;
    } else {
      matrix.setup(solver_num_col, solver_num_row,
		     &solver_lp.Astart_[0],
		     &solver_lp.Aindex_[0],
		     &solver_lp.Avalue_[0],
		     &basis.nonbasicFlag_[0]);
      //      printf("Called matrix_->setup\n");cout<<flush;
    }
    // Indicate that there is a colum-wise and row-wise copy of the
    // matrix: can't be done in matrix_->setup_lgBs
    //    simplex_info.solver_lp_has_matrix_col_wise = true;
    //    simplex_info.solver_lp_has_matrix_row_wise = true;
  }

    // TODO Put something in to skip factor_->setup
    // Initialise factor arrays, passing &basis.basicIndex_[0] so that its
    // address can be copied to the internal Factor pointer
    factor.setup(solver_num_col, solver_num_row,
		   &solver_lp.Astart_[0],
		   &solver_lp.Aindex_[0],
		   &solver_lp.Avalue_[0],
		   &basis.basicIndex_[0]);
    // Indicate that the model has factor arrays: can't be done in factor.setup
    //simplex_info.solver_lp_has_factor_arrays = true;
  }

  void flip_bound(HighsModelObject &highs_model_object, int iCol) {
    int *nonbasicMove = &highs_model_object.basis_.nonbasicMove_[0];
    HighsSimplexInfo &simplex_info = highs_model_object.simplex_info_;
    const int move = nonbasicMove[iCol] = -nonbasicMove[iCol];
    simplex_info.workValue_[iCol] = move == 1 ? simplex_info.workLower_[iCol] : simplex_info.workUpper_[iCol];
  }

  /*
  // Increment iteration count (here!) and (possibly) store the pivots for
  // debugging NLA
  void record_pivots(int columnIn, int columnOut, double alpha) {
    // NB This is where the iteration count is updated!
    if (columnIn >= 0) simplex_info_.iteration_count++;
#ifdef HiGHSDEV
    historyColumnIn.push_back(columnIn);
    historyColumnOut.push_back(columnOut);
    historyAlpha.push_back(alpha);
#endif
  }
#ifdef HiGHSDEV
  // Store and write out the pivots for debugging NLA
  void writePivots(const char* suffix) {
    string filename = "z-" + solver_lp_->model_name_ + "-" + suffix;
    ofstream output(filename.c_str());
    int count = historyColumnIn.size();
    double current_run_highs_time = timer_->readRunHighsClock();
    output << solver_lp_->model_name_ << " " << count << "\t" << current_run_highs_time << endl;
    output << setprecision(12);
    for (int i = 0; i < count; i++) {
      output << historyColumnIn[i] << "\t";
      output << historyColumnOut[i] << "\t";
      output << historyAlpha[i] << endl;
    }
    output.close();
  }
#endif
  */
};
#endif // SIMPLEX_HSIMPLEX_H_
