#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Link the UI milestone driver against an existing Ninja GUI build and run it.

Build openrct2 first. For cross builds, --runner-prefix can specify QEMU and the
matching target loader/library path. Captures use production UI and native fixture state, not physical device screenshots.
RideFlowTests additionally renders the native park using privately installed RCT2 assets.
"""
import argparse
from pathlib import Path
import os
import shlex
import shutil
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--driver", choices=["UiRedesignTests", "ConstructionUxTests", "RideFlowTests", "PeopleUxTests", "PathBatchTests", "ManagementUxTests", "EverydayControlsTests", "PathReliabilityTests", "PathCursorTests", "PauseResumeTests", "SceneryPathParityTests"], default="UiRedesignTests")
parser.add_argument("--build", type=Path, required=True)
parser.add_argument("--data", type=Path, required=True)
parser.add_argument("--profile", type=Path, required=True)
parser.add_argument("--captures", type=Path, required=True)
parser.add_argument("--rct2-assets", type=Path, help="Private installed RCT2 root containing Data/g1.dat")
parser.add_argument("--ninja", default=shutil.which("ninja") or "ninja")
parser.add_argument("--runner-prefix", default="")
parser.add_argument("--size", nargs=2, type=int, metavar=("WIDTH", "HEIGHT"),
                    help="Physical window dimensions for UiRedesignTests")
args = parser.parse_args()
if args.size and (args.driver != "UiRedesignTests" or min(args.size) < 1):
    parser.error("--size requires positive dimensions and the UiRedesignTests driver")
if args.driver in ("RideFlowTests", "PeopleUxTests", "PathBatchTests", "ManagementUxTests") and (not args.rct2_assets or not (args.rct2_assets / "Data/g1.dat").is_file()):
    parser.error("Asset-backed drivers require --rct2-assets with an installed Data/g1.dat")
repo = Path(__file__).resolve().parents[2]
build = args.build.resolve()
commands = subprocess.check_output(
    [args.ninja, "-C", str(build), "-t", "commands", "openrct2"], text=True
).splitlines()
compile_args = shlex.split(next(c for c in commands if "/handheld/Explorer.cpp" in c and " -c " in c))
includes = []
for i, value in enumerate(compile_args):
    if value.startswith(("-I", "-D")):
        includes.append(value)
    elif value == "-isystem":
        includes.extend((value, compile_args[i + 1]))
link = shlex.split(commands[-1])
# Ninja commands may include a compiler launcher such as ccache. Invoke the
# configured compiler directly so the driver flags are passed to the compiler.
compiler = next(
    line.split("=", 1)[1]
    for line in (build / "CMakeCache.txt").read_text().splitlines()
    if line.startswith("CMAKE_CXX_COMPILER:")
)
start = link.index(compiler)
end = link.index("&&", start) if "&&" in link[start:] else len(link)
link = [v for v in link[start:end] if not v.endswith("/Ui.cpp.o")]
out = link.index("-o")
binary = build / {"UiRedesignTests": "handheld-ui-redesign-tests", "ConstructionUxTests": "handheld-construction-ux-tests",
                  "RideFlowTests": "handheld-ride-flow-tests", "PeopleUxTests": "handheld-people-ux-tests", "PathBatchTests": "handheld-path-batch-tests", "ManagementUxTests": "handheld-management-ux-tests", "EverydayControlsTests": "handheld-everyday-controls-tests", "PathReliabilityTests": "handheld-path-reliability-tests", "PathCursorTests": "handheld-path-cursor-tests", "PauseResumeTests": "handheld-pause-resume-tests", "SceneryPathParityTests": "handheld-scenery-path-parity-tests"}[args.driver]
link[out + 1] = str(binary)
link.insert(out, str(repo / "test/handheld" / (args.driver + ".cpp")))
link[1:1] = ["-std=c++20", "-fno-char8_t", *includes, f"-I{repo / 'src'}", f"-I{repo / 'src/thirdparty'}"]
subprocess.run(link, cwd=build, check=True)
env = os.environ.copy()
env.update(SDL_VIDEODRIVER="offscreen", SDL_AUDIODRIVER="dummy")
if args.driver in ("RideFlowTests", "PeopleUxTests", "PathBatchTests", "ManagementUxTests", "EverydayControlsTests", "PathReliabilityTests", "PathCursorTests", "SceneryPathParityTests"):
    env[{"RideFlowTests": "POCKET_RIDE_FLOW_CAPTURES", "PeopleUxTests": "POCKET_PEOPLE_UX_CAPTURES", "PathBatchTests": "POCKET_PATH_BATCH_CAPTURES", "ManagementUxTests": "POCKET_MANAGEMENT_UX_CAPTURES", "EverydayControlsTests": "POCKET_PATH_BATCH_CAPTURES", "PathReliabilityTests": "POCKET_PATH_BATCH_CAPTURES", "PathCursorTests": "POCKET_PATH_BATCH_CAPTURES", "SceneryPathParityTests": "POCKET_SCENERY_PATH_CAPTURES"}[args.driver]] = str(args.captures.resolve())
fixture_input = args.rct2_assets if args.driver in ("RideFlowTests", "PeopleUxTests", "PathBatchTests", "ManagementUxTests", "EverydayControlsTests", "PathReliabilityTests", "PathCursorTests", "SceneryPathParityTests") else args.captures
subprocess.run(
    [*shlex.split(args.runner_prefix), str(binary), str(args.data.resolve()),
     str(args.profile.resolve()), *([] if args.driver == "PauseResumeTests" else [str(fixture_input.resolve())]),
     *(str(dimension) for dimension in args.size or ())], env=env, check=True
)
