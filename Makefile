push:
	rsync -uav -e "ssh -p 22" --exclude=".idea" --exclude="testdir"  --exclude=".git" --exclude="cmake-build-debug" ./ ${TARGET}
