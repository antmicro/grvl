#/bin/env bash

SAMPLE_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BUILD_ALL=0
BUILD_CLEAN=0
BUILD_ONLY=0

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
    --help)
      echo "sample.sh [--build|--clean|--help] --all|<sample_name>"
      echo " --build  Do not run the sample after building"
      echo " --clean  Do a clean build of the sample"
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

    if [ "$BUILD_CLEAN" -eq "1" ] && [ -d "$SAMPLE_PATH/build" ]; then
        rm -rf "$SAMPLE_PATH/build"
    fi

    set -e
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B $SAMPLE_PATH/build/ $SAMPLE_PATH
    cmake --build $SAMPLE_PATH/build/ -j $(nproc)

    if [ ! "$BUILD_ONLY" -eq "1" ]; then
        echo "Running sample '$1'..."
        $SAMPLE_PATH/build/demo
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
