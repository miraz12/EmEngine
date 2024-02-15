enum KEY {
	escape
	w
	a
	s
	d
	f
	o
	space
	arrowup
	arrowdown
	arrowright
	arrowleft
	mouse1
}

fn C.Input_getPressed(vec &&int) int

pub fn update_input() {
	v := &int(0)
	size := C.Input_getPressed(&v)

	if size > 0 {
		unsafe {
			for i in 0 .. size {
				match KEY(v[i]) {
					.escape {
						println('[input] ESC')
					}
					.w {
						println('[input] w')
					}
					.a {
						println('[input] a')
					}
					.s {
						println('[input] s')
					}
					.d {
						println('[input] d')
					}
					.f {
						println('[input] f')
					}
					.o {
						println('[input] o')
					}
					.space {
						println('[input] space')
					}
					.arrowup {
						println('[input] arrow up')
					}
					.arrowdown {
						println('[input] arrow down')
					}
					.arrowright {
						println('[input] arrow right')
					}
					.arrowleft {
						println('[input] arrow left')
					}
					.mouse1 {
						println('[input] mouse1')
					}
				}
			}
		}
	}
}
