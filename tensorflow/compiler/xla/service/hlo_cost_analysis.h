/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_COMPILER_XLA_SERVICE_HLO_COST_ANALYSIS_H_
#define TENSORFLOW_COMPILER_XLA_SERVICE_HLO_COST_ANALYSIS_H_

#include <functional>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "tensorflow/compiler/xla/service/dfs_hlo_visitor.h"
#include "tensorflow/compiler/xla/service/hlo_computation.h"
#include "tensorflow/compiler/xla/service/hlo_instruction.h"
#include "tensorflow/compiler/xla/shape_util.h"
#include "tensorflow/compiler/xla/statusor.h"
#include "tensorflow/compiler/xla/xla_data.pb.h"

namespace xla {

// HloCostAnalysis traverses an HLO graph and calculates the amount of
// computations required for the graph. Each HLO instruction handler provides
// the computation cost of the instruction, and the values are accumulated
// during the traversal for the entire graph. We treat normal floating point
// operations separately from transcendental operations.
class HloCostAnalysis : public ConstDfsHloVisitor {
 public:
  // Each HLO is associated to a vector of properties with the indices given
  // below. Sub-classes can add further properties.
  // MSVC 14.0 limitation requires the consts.
  typedef std::map<std::string, float, std::less<>> Properties;
  // shape_size is a function which returns the size in bytes of the top-level
  // buffer of a shape.
  using ShapeSizeFunction = std::function<int64_t(const Shape&)>;

  static constexpr const char kFlopsKey[] = "flops";
  static constexpr const char kTranscendentalsKey[] = "transcendentals";
  static constexpr const char kBytesAccessedKey[] = "bytes accessed";
  static constexpr const char kOptimalSecondsKey[] = "optimal_seconds";
  static constexpr const char kUtilizationKey[] = "utilization";

  // A struct to encapsulate hardware-related options. This includes the shape
  // size function, which is used to encode hardware-specific padding and per
  // second rates of FLOPs, bytes per second (available bandwidth), and
  // transcendentals per second.
  struct Options {
    // Function which computes the size of the top-level of a given shape (not
    // including nested elements, if any). If null then bytes_accessed methods
    // return an error.
    ShapeSizeFunction shape_size;
    // How much of each property can be processed per second. E.g. if the
    // property is bytes accessed, this is the number of bytes that can be
    // processed per second. Is empty if no rates have been set.
    Properties per_second_rates = {};

    // Set the rates used to calculate the time taken by the computation.
    void set_flops_per_second(float value) {
      per_second_rates[kFlopsKey] = value;
    }
    void set_transcendentals_per_second(float value) {
      per_second_rates[kTranscendentalsKey] = value;
    }
    void set_bytes_per_second(float value) {
      per_second_rates[kBytesAccessedKey] = value;
    }

    // Returns the specified per-second rate used by cost analysis.
    const float per_second_rate(const std::string& key) const {
      return GetProperty(key, per_second_rates);
    }
  };

  explicit HloCostAnalysis(const Options& options);
  explicit HloCostAnalysis(ShapeSizeFunction shape_size,
                           const Properties& per_second_rates = {});

  Status HandleElementwiseUnary(const HloInstruction* hlo) override;
  Status HandleElementwiseBinary(const HloInstruction* hlo) override;
  Status HandleConstant(const HloInstruction* constant) override;
  Status HandleIota(const HloInstruction* iota) override;
  Status HandleGetTupleElement(
      const HloInstruction* get_tuple_element) override;
  Status HandleSelect(const HloInstruction* hlo) override;
  Status HandleCompare(const HloInstruction* compare) override;
  Status HandleClamp(const HloInstruction* clamp) override;
  Status HandleReducePrecision(const HloInstruction* hlo) override;
  Status HandleConcatenate(const HloInstruction* concatenate) override;
  Status HandleAsyncStart(const HloInstruction* async_start) override;
  Status HandleAsyncUpdate(const HloInstruction* async_update) override;
  Status HandleAsyncDone(const HloInstruction* async_done) override;
  Status HandleCopyStart(const HloInstruction* send) override;
  Status HandleCopyDone(const HloInstruction* send_done) override;
  Status HandleSend(const HloInstruction* send) override;
  Status HandleSendDone(const HloInstruction* send_done) override;
  Status HandleRecv(const HloInstruction* recv) override;
  Status HandleRecvDone(const HloInstruction* recv_done) override;
  Status HandleConvert(const HloInstruction* convert) override;
  Status HandleCopy(const HloInstruction* copy) override;
  Status HandleDomain(const HloInstruction* domain) override;
  Status HandleDot(const HloInstruction* dot) override;
  Status HandleConvolution(const HloInstruction* convolution) override;
  Status HandleFft(const HloInstruction* fft) override;
  Status HandleTriangularSolve(const HloInstruction* hlo) override;
  Status HandleCholesky(const HloInstruction* hlo) override;
  Status HandleOptimizationBarrier(const HloInstruction* hlo) override;
  Status HandleAllGather(const HloInstruction* hlo) override;
  Status HandleAllGatherStart(const HloInstruction* hlo) override;
  Status HandleAllGatherDone(const HloInstruction* hlo) override;
  Status HandleAllReduce(const HloInstruction* crs) override;
  Status HandleReduceScatter(const HloInstruction* hlo) override;
  Status HandleAllReduceStart(const HloInstruction* hlo) override;
  Status HandleAllReduceDone(const HloInstruction* hlo) override;
  Status HandleAllToAll(const HloInstruction* hlo) override;
  Status HandleCollectivePermute(const HloInstruction* hlo) override;
  Status HandleCollectivePermuteStart(const HloInstruction* hlo) override;
  Status HandleCollectivePermuteDone(const HloInstruction* hlo) override;
  Status HandleReplicaId(const HloInstruction* hlo) override;
  Status HandlePartitionId(const HloInstruction* hlo) override;
  Status HandleInfeed(const HloInstruction* infeed) override;
  Status HandleOutfeed(const HloInstruction* outfeed) override;
  Status HandleRng(const HloInstruction* random) override;
  Status HandleRngBitGenerator(const HloInstruction* random) override;
  Status HandleRngGetAndUpdateState(const HloInstruction* random) override;
  Status HandleReverse(const HloInstruction* reverse) override;
  Status HandleSort(const HloInstruction* sort) override;
  Status HandleParameter(const HloInstruction* parameter) override;
  Status HandleReduce(const HloInstruction* reduce) override;
  Status HandleBatchNormTraining(
      const HloInstruction* batch_norm_training) override;
  Status HandleBatchNormInference(
      const HloInstruction* batch_norm_inference) override;
  Status HandleBatchNormGrad(const HloInstruction* batch_norm_grad) override;
  Status HandleFusion(const HloInstruction* fusion) override;
  Status HandleCall(const HloInstruction* call) override;
  Status HandleCustomCall(const HloInstruction* custom_call) override;
  Status HandleSlice(const HloInstruction* slice) override;
  Status HandleDynamicSlice(const HloInstruction* dynamic_slice) override;
  Status HandleDynamicUpdateSlice(
      const HloInstruction* dynamic_update_slice) override;
  Status HandleTuple(const HloInstruction* tuple) override;
  Status HandleMap(const HloInstruction* map) override;
  Status HandleReduceWindow(const HloInstruction* reduce_window) override;
  Status HandleSelectAndScatter(const HloInstruction* instruction) override;
  Status HandleBitcast(const HloInstruction* bitcast) override;
  Status HandleBroadcast(const HloInstruction* broadcast) override;
  Status HandlePad(const HloInstruction* pad) override;
  Status HandleReshape(const HloInstruction* reshape) override;
  Status HandleDynamicReshape(const HloInstruction* reshape) override;
  Status HandleAddDependency(const HloInstruction* add_dependency) override;
  Status HandleAfterAll(const HloInstruction* token) override;
  Status HandleTranspose(const HloInstruction* transpose) override;
  Status HandleWhile(const HloInstruction* xla_while) override;
  Status HandleConditional(const HloInstruction* conditional) override;
  Status HandleGather(const HloInstruction* gather) override;
  Status HandleScatter(const HloInstruction* hlo) override;
  Status HandleGetDimensionSize(const HloInstruction* get_size) override;
  Status HandleSetDimensionSize(const HloInstruction* set_size) override;
  Status FinishVisit(const HloInstruction* root) override;

  Status Preprocess(const HloInstruction* hlo) override;
  Status Postprocess(const HloInstruction* hlo) override;

  // Enable efficient updates if a known small set of instructions within an
  // HLO graph was modified.
  // Updates the cost analysis by removing one instruction.
  Status RemoveInstruction(HloInstruction* instruction);
  // Updates the cost analysis by re-doing the analysis of one instruction.
  Status RevisitInstruction(HloInstruction* instruction);

  // Decorates shape_size_ by returning 0 immediately if the shape does not have
  // a layout.
  int64_t GetShapeSize(const Shape& shape) const;

  // Returns properties for the computation.
  float flop_count() const;
  float transcendental_count() const;
  float bytes_accessed() const;
  float optimal_seconds() const;

  // Returns the respective cost computed for a particular HLO instruction, or 0
  // if the HLO was not found to have a cost in the analysis.
  //
  // Note that the cost for sub HLO instructions are also returned if asked. For
  // example, body and condition of a while, fused instructions within a
  // fusion, or the add instruction of a reduce.
  int64_t flop_count(const HloInstruction& hlo) const;
  int64_t transcendental_count(const HloInstruction& hlo) const;
  int64_t bytes_accessed(const HloInstruction& hlo) const;
  int64_t operand_bytes_accessed(const HloInstruction& hlo, int64_t operand_num,
                                 ShapeIndex index = {}) const;
  // Value indicating how much each input of the instruction
  // is used assuming its output is fully used.
  // This is 1.0 for most cases except operations involving slicing (<1)
  // and on some backends in addition reuse of inputs (>1).
  float operand_utilization(const HloInstruction& hlo, int64_t operand_num,
                            ShapeIndex index = {}) const;
  int64_t output_bytes_accessed(const HloInstruction& hlo,
                                ShapeIndex index = {}) const;
  float optimal_seconds(const HloInstruction& hlo) const;

  // Get bytes read/written by this HLO. If memory_space is provided, it returns
  // the bytes read/written from/to the given memory space only.
  int64_t GetBytesRead(
      const HloInstruction& hlo,
      std::optional<int64_t> memory_space = std::nullopt) const;
  int64_t GetBytesWritten(
      const HloInstruction& hlo,
      std::optional<int64_t> memory_space = std::nullopt) const;

  const Properties& properties() const { return properties_sum_; }
  const float property(const std::string& key) const {
    return GetProperty(key, properties());
  }

  // Returns the specified per-second rate used by cost analysis.
  const float per_second_rate(absl::string_view key) const {
    return GetProperty(key, options_.per_second_rates);
  }

  // Return the key that is used to index into Properties for the specified
  // input/output at the shape index.
  static std::string GetOperandBytesAccessedKey(int64_t operand_num,
                                                ShapeIndex index = {});
  static std::string GetOperandUtilizationKey(int64_t operand_num,
                                              ShapeIndex index = {});
  static std::string GetOutputBytesAccessedKey(ShapeIndex index = {});

  // Returns the estimated convolution flops.
  virtual int64_t GetConvolutionFlops(const HloInstruction* convolution);
  // Same as above but with parameters for shapes to allow for backends to
  // refine these.
  static int64_t GetConvolutionFlops(const HloInstruction* convolutions,
                                     const Shape& lhs_shape,
                                     const Shape& rhs_shape,
                                     const Shape& result_shape);

  // Returns the estimated dot flops.
  static int64_t GetDotFlops(const Shape& lhs_shape, const Shape& result_shape,
                             const DotDimensionNumbers& dnums);

 protected:
  typedef absl::flat_hash_map<const HloInstruction*, Properties>
      HloToProperties;

  // An FMA counts as two floating point operations in these analyzes.
  static constexpr int64_t kFmaFlops = 2;

  // Operations like broadcast with reused inputs are
  // not handled efficiently on some platforms.
  virtual bool input_reuse_is_inefficient() const { return false; }

  // Small constants can be embedded in the assembly and not require
  // memory access.
  virtual size_t immediate_constant_max_elements() const { return 1; }

  // Creates a nested instance of HloCostAnalysis using the same Options.
  virtual std::unique_ptr<HloCostAnalysis> CreateNestedCostAnalysis();

  // Returns the properties computed from visiting the computation rooted at the
  // given hlo. The cost of visited sub HLO instructions is saved to
  // hlo_properties_, which will be used by functions such as
  // flop_count(hlo_instruction) to return cost of a particular HLO instruction.
  StatusOr<Properties> ProcessSubcomputation(HloComputation* computation);

  // Utility function to handle all element-wise operations.
  Status HandleElementwiseOp(const HloInstruction* hlo_instruction);

  // Returns the default value if the key is not present in the
  // properties. Otherwise, returns the value that the key maps to from the
  // properties parameter.
  static float GetProperty(absl::string_view key, const Properties& properties,
                           float default_value = 0.0f);

  // Returns 0.0f if the hlo is not present in hlo_to_properties or if the key
  // is not present in hlo_to_properties[hlo]. Otherwise, returns the value that
  // the key maps to in the properties of the given hlo.
  static float GetPropertyForHlo(const HloInstruction& hlo,
                                 const std::string& key,
                                 const HloToProperties& hlo_to_properties);

  // Traverses a fusion operand to find the actual bytes accessed by the fusion
  // node.
  virtual int64_t FusionParameterReadBytes(const HloInstruction* hlo) const;

  // Traverses a fusion counting total utilization of every instruction inside.
  // Currently implemented non-trivially only in the GPU cost analysis.
  virtual Status FusionCalculateUtilizations(const HloInstruction* fusion);

  // Set bytes accessed by the specified operand and shape index.
  void SetOperandBytesAccessed(int64_t operand_num, float value);
  void SetOperandBytesAccessed(int64_t operand_num, ShapeIndex index,
                               float value);

  void SetOperandUtilization(int64_t operand_num, float value);
  void SetOperandUtilization(int64_t operand_num, ShapeIndex index,
                             float value);

  // Set bytes accessed by the output at the shape index.
  void SetOutputBytesAccessed(float value);
  void SetOutputBytesAccessed(ShapeIndex index, float value);

  HloToProperties hlo_properties_;

  // If true, the time taken will be computed from the rates for each property
  // and the total time will be the maximum time, which is the time of the
  // bottleneck.
  bool current_should_compute_bottleneck_time_;

  // The properties of the currently visited instruction. A HandleFoo method can
  // modify these to change the default values computed in Preprocess.
  Properties current_properties_;

  // The sum of the properties of all HLOs in the computation.
  Properties properties_sum_;

  // The hardware-specific options that contains things like the shape size
  // function and per-second rates.
  Options options_;

  // Determines which properties propagate from subcomputations to parents.
  static bool KeyToCopyFromSubcomputation(absl::string_view key);

  HloCostAnalysis(const HloCostAnalysis&) = delete;
  HloCostAnalysis& operator=(const HloCostAnalysis&) = delete;
};

}  // namespace xla

#endif  // TENSORFLOW_COMPILER_XLA_SERVICE_HLO_COST_ANALYSIS_H_
