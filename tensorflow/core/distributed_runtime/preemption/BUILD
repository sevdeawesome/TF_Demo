load("//tensorflow:tensorflow.bzl", "tf_cc_test", "tf_gen_op_libs", "tf_gen_op_wrapper_cc", "tf_gen_op_wrapper_py", "tf_kernel_library")
load("//tensorflow:tensorflow.default.bzl", "tf_grpc_cc_dependencies")

package(
    default_visibility = [
        "//tensorflow:internal",
    ],
    licenses = ["notice"],
)

cc_library(
    name = "preemption_sync_manager",
    srcs = ["preemption_sync_manager.cc"],
    hdrs = ["preemption_sync_manager.h"],
    deps = [
        "//tensorflow/core/distributed_runtime:call_options",
        "//tensorflow/core/distributed_runtime/coordination:coordination_service_agent",
        "//tensorflow/core/platform",
        "//tensorflow/core/platform:env",
        "//tensorflow/core/platform:mutex",
        "//tensorflow/core/platform:status",
        "//tensorflow/core/platform:statusor",
        "//tensorflow/tsl/distributed_runtime/preemption:preemption_notifier",
        "//tensorflow/tsl/protobuf:coordination_service_proto_cc",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
    ],
)

tf_cc_test(
    name = "preemption_sync_manager_test",
    size = "small",
    srcs = ["preemption_sync_manager_test.cc"],
    deps = [
        ":preemption_sync_manager",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/time",
        "//tensorflow/core:test",
        "//tensorflow/core:test_main",
        "//tensorflow/core/distributed_runtime/coordination:coordination_client",
        "//tensorflow/core/distributed_runtime/coordination:coordination_service_agent",
        "//tensorflow/core/distributed_runtime/rpc/coordination:grpc_coordination_client",
        "//tensorflow/core/distributed_runtime/rpc/coordination:grpc_coordination_service_impl",
        "//tensorflow/core/platform:env",
        "//tensorflow/core/platform:status",
        "//tensorflow/core/platform:errors",
        "//tensorflow/core/platform:statusor",
        "//tensorflow/tsl/distributed_runtime/coordination:coordination_service",
        "//tensorflow/tsl/distributed_runtime/coordination:coordination_service_impl",
        "//tensorflow/tsl/distributed_runtime/preemption:preemption_notifier",
        "//tensorflow/tsl/distributed_runtime/rpc:async_service_interface",
        "//tensorflow/tsl/protobuf:coordination_config_proto_cc",
    ] + tf_grpc_cc_dependencies(),
)

cc_library(
    name = "check_preemption_op",
    srcs = ["check_preemption_op.cc"],
    deps = ["//tensorflow/core:framework"],
    alwayslink = 1,
)

tf_kernel_library(
    name = "check_preemption_op_kernel",
    srcs = ["check_preemption_op_kernel.cc"],
    deps = [
        "//tensorflow/core:framework",
        "//tensorflow/core:lib",
        "//tensorflow/core/distributed_runtime:error_payloads",
        "//tensorflow/core/distributed_runtime/coordination:coordination_service_agent",
        "@com_google_absl//absl/strings",
    ],
    alwayslink = 1,
)

tf_gen_op_libs(
    op_lib_names = ["check_preemption_op"],
    sub_directory = "",
    deps = ["//tensorflow/core:lib"],
)

tf_gen_op_wrapper_cc(
    name = "gen_cc_check_preemption_op",
    out_ops_file = "cc/check_preemption_op",
    deps = [":check_preemption_op_op_lib"],
)

cc_library(
    name = "check_preemption_ops_cc",
    srcs = ["cc/check_preemption_op.cc"],
    hdrs = ["cc/check_preemption_op.h"],
    deps = [
        ":check_preemption_op_op_lib",
        "//tensorflow/cc:const_op",
        "//tensorflow/cc:ops",
        "//tensorflow/cc:scope",
        "//tensorflow/core:framework",
        "//tensorflow/core:lib",
    ],
)

tf_gen_op_wrapper_py(
    name = "gen_check_preemption_op",
    out = "gen_check_preemption_op.py",
    deps = [":check_preemption_op_op_lib"],
)
