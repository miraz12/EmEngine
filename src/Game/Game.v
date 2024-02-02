import os

// Declare the C function
fn C.ECS_loadScene(file &char)

@[export: 'Game_initialize']
fn initialize() {
	println('[game] initialize')
	scene_file := os.resource_abs_path('../../resources/scene.yaml')
	C.ECS_loadScene(scene_file.str)
}

@[export: 'Game_update']
fn update(dt f32) {
	println('[game] update')
}
