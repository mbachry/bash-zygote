all: build/bash-zygote-client build/bash-zygote

build/build.ninja: meson.build
	meson setup build

build/bash-zygote-client: build/build.ninja zygote-client.c
	ninja -C build

build/bash-zygote: bash/Makefile
	make -C bash
	cp bash/bash build/bash-zygote

bash/Makefile:
	@if ! grep -q varlink bash/Makefile.in; then \
	  ( cd bash && patch -p1 < ../bash.diff ); \
	fi
	( cd bash && ./configure --prefix=/usr )

clean:
	rm -rf build
	make -C bash clean
