#! /bin/sh -x 
git_tree_ish=$1
debian_dir=$(dirname $0)
git_dir=$(mktemp -td k4dirstat.XXX)
version=$(dpkg-parsechangelog -l$debian_dir/changelog | sed -ne 's/^Version: \(\([0-9]\+\):\)\?\(.*\)-.*/\3/p')
#remote git archive does not work with this repository so we must clone first
git clone --bare http://grumpypenguin.org/~josh/kdirstat.git $git_dir
git --git-dir=$git_dir archive --prefix=k4dirstat-$version/ $git_tree_ish | gzip > k4dirstat_$version.orig.tar.gz
#Keep the tmp bare repository for conveniance
