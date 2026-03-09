const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const libfractions_mod = b.createModule(.{
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const libfractions = b.addLibrary(.{
        .name = "fractions",
        .linkage = .static,
        .root_module = libfractions_mod,
    });
    libfractions.root_module.addCSourceFiles(.{ .files = &.{
        "src/fractions.c",
    }, .flags = &.{
        "-Wall",
        "-Wextra",
        "-g",
    } });
    libfractions.root_module.addIncludePath(b.path("include"));

    b.installArtifact(libfractions);

    const exe = b.addExecutable(.{ .name = "main", .root_module = b.createModule(.{
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    }) });

    exe.root_module.addCSourceFiles(.{ .files = &.{
        "main.c",
    }, .flags = &.{
        "-Wall",
        "-Wextra",
        "-g",
    } });

    exe.root_module.addIncludePath(b.path("include"));

    exe.root_module.linkLibrary(libfractions);

    b.installArtifact(exe);
}
