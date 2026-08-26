#/bin/env bash

SAMPLE_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BUILD_ALL=0
BUILD_CLEAN=0
BUILD_ONLY=0
BUILD_TYPE=RelWithDebInfo
BUILD_DIR="build"
INTERPRETER=""

while [[ $# -gt 0 ]]; do
  case $1 in
    --all)
      BUILD_ALL=1
      shift
      ;;
    --clean)
      BUILD_CLEAN=1
      shift
      ;;
    --build)
      BUILD_ONLY=1
      shift
      ;;
    --debug)
      BUILD_TYPE=Debug
      BUILD_DIR="build-debug"
      INTERPRETER="gdb"
      shift
      ;;
    --help)
      echo "sample.sh [--build|--clean|--debug|--help] --all|<sample_name>"
      echo " --build  Do not run the sample after building"
      echo " --clean  Do a clean build of the sample"
      echo " --debug  Do a Debug build (default is RelWithDebInfo) and run in GDB"
      echo " --all    Build all samples"
      echo " --help   Print this help page and exit"
      exit 0
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      SAMPLE_NAME="$1"
      shift
      ;;
  esac
done

function sample() {
    SAMPLE_PATH="$SAMPLE_DIR/$1"

    if [ ! -d "$SAMPLE_PATH" ]; then
        echo "Sample '$1' does not exist."
        echo "(Looked in '$SAMPLE_DIR')"
        exit 1
    fi

    echo "Building sample '$1'..."
    rm -f "$SAMPLE_PATH/$BUILD_DIR/demo" || true

    if [ "$BUILD_CLEAN" -eq "1" ] && [ -d "$SAMPLE_PATH/$BUILD_DIR" ]; then
        rm -rf "$SAMPLE_PATH/$BUILD_DIR"
    fi

    set -e
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -B "$SAMPLE_PATH/$BUILD_DIR/" $SAMPLE_PATH
    cmake --build "$SAMPLE_PATH/$BUILD_DIR/" -j $(nproc)

    if [ ! -f "$SAMPLE_PATH/$BUILD_DIR/demo" ]; then
        echo "Failed to build sample $SAMPLE_PATH, executable file not found!"
        exit 1
    fi

    if [ ! "$BUILD_ONLY" -eq "1" ]; then
        echo "Running sample '$1'..."
        eval $INTERPRETER "$SAMPLE_PATH/$BUILD_DIR/demo"
    fi
}

if [ "$BUILD_ALL" -eq "1" ]; then
    for dir in $(find "$SAMPLE_DIR" -maxdepth 1 -mindepth 1 -type d -printf '%f\n')
    do
        sample "$dir"
    done

    exit 0
fi

sample $SAMPLE_NAME
