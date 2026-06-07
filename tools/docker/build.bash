#!/bin/bash
#
# Script for building image
#
# ./build.bash docker-folder [git-branch]
#
# Requires a file $HOME/.gitlab_access_token containing git
# credentials in the form <user>:<password> or <key>:<token>

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Stop script execution on error
set -e

DOCKER_FOLDER=$1
GIT_BRANCH=$2

if [[ -z "$DOCKER_FOLDER" ]]
then
    DOCKER_FOLDER=ubuntu-26.04
    echo "No docker folder specified; building $DOCKER_FOLDER"
fi

if [[ -z "$GIT_BRANCH" ]]
then
    GIT_BRANCH=main
fi
GIT_COMMIT=`git rev-parse $GIT_BRANCH`

commit_date=$(git log -1 --format=%cd --date=format:'%Y-%m-%d' $GIT_BRANCH)
commit_hash=$(git rev-parse --short $GIT_BRANCH)
branch_name=$(git rev-parse --abbrev-ref $GIT_BRANCH | tr '/' '-')

DOCKER_TAG=${DOCKER_FOLDER}_${commit_date}_${commit_hash}-${branch_name}

echo
echo "-------------------------------------------------------------"
echo "----------------    BUILDING DOCKER IMAGE    ----------------"
echo "BRANCH: $GIT_BRANCH"
echo "COMMIT: $GIT_COMMIT"
echo "TAG:    $DOCKER_TAG"
echo

CUR_BRANCH=`git branch | grep "*" | cut -c 3-`
if [[ "$GIT_BRANCH" != "$CUR_BRANCH" ]]
then
    echo -e "\033[33;40;1mWARNING: building $GIT_BRANCH while on $CUR_BRANCH\033[0m"
    echo
    read -p "Are you sure? (y/n)" -n 1 -r
    echo

    if [[ ! $REPLY =~ ^[Yy]$ ]]
    then
        echo " *Cancelled*"
        exit 1
    fi
fi

echo "Building $DOCKER_FOLDER"
docker build \
  -t $DOCKER_TAG \
  --secret id=GIT_AUTH,src=$HOME/.gitlab_utm50_access_token \
  --build-arg GIT_BRANCH=$GIT_BRANCH \
  --build-arg GIT_COMMIT=$GIT_COMMIT \
  $DIR/$DOCKER_FOLDER

echo
echo "--------------------------- DONE ----------------------------"
echo "BRANCH: $GIT_BRANCH"
echo "COMMIT: $GIT_COMMIT"
echo "TAG:    $DOCKER_TAG"
echo "-------------------------------------------------------------"
echo
