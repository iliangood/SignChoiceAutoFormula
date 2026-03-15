const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // const libfractions_mod =
    const libfractions = b.addLibrary(.{
        .name = "fractions",
        .linkage = .static,
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/fractions.zig"),
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        }),
    });

    b.installArtifact(libfractions);

    const exe_mod = b.createModule(.{
        .root_source_file = b.path("main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const exe = b.addExecutable(.{
        .name = "main",
        .root_module = exe_mod,
    });

    exe.root_module.linkLibrary(libfractions);
    exe.root_module.addImport("fractions", libfractions.root_module);

    const measure_time = b.option(bool, "measure-time", "measure time") orelse false;
    const options = b.addOptions();
    options.addOption(bool, "measure_time", measure_time);
    exe.root_module.addOptions("config", options);

    b.installArtifact(exe);

    const cmd_run = b.addRunArtifact(exe);
    cmd_run.step.dependOn(b.getInstallStep());
    if (b.args) |args| cmd_run.addArgs(args);

    const run_step = b.step("run", "rum main");
    run_step.dependOn(&cmd_run.step);

    const tests_step = b.step("test", "run tests");

    const unit_tests_main = b.addTest(.{
        .root_module = exe_mod,
    });
    const run_unit_tests_main = b.addRunArtifact(unit_tests_main);
    tests_step.dependOn(&run_unit_tests_main.step);
    const unit_tests_libfractions = b.addTest(.{
        .root_module = libfractions.root_module,
    });
    const run_unit_tests_libfractions = b.addRunArtifact(unit_tests_libfractions);
    tests_step.dependOn(&run_unit_tests_libfractions.step);
}
