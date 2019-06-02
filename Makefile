all:
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j8 && mv src/server ../netstore-server && mv src/client ../netstore-client
clean:
	rm -rf build netstore-server netstore-client
push:
	rsync -uav -e "ssh -p 22" --exclude=".idea" --exclude="testdir"  --exclude=".git" --exclude="cmake-build-debug" ./ ${TARGET}
package:
	mkdir -p sa391211 && cp -r src test CMakeLists.txt Makefile helpers.sh .gitignore sa391211 && \
	tar -zcvf sa391211.tar.gz sa391211 && rm -r sa391211