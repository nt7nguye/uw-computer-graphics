
gold = gr.material(	 {0.9, 0.8, 0.4}, 	{0.8, 0.8, 0.4}, 	25, 0.0)
gold2 = gr.material( {1.0, 1.0, 0.3},   {1.0, 1.0, 1.0},	25, 0.0)
grass = gr.material( {0.1, 0.7, 0.1}, 	{0.0, 0.0, 0.0}, 	0,  0.0)
blue = gr.material(  {0.2, 0.2, 0.9}, 	{0.1, 0.1, 0.5}, 	25, 1e-5)
glass = gr.material( {0.05, 0.05, 0.05}, {0.7, 0.7, 0.7}, 25, 1.5)			--enable optics
water = gr.material( {0.05, 0.05, 0.05}, {0.6, 0.6, 0.6}, 25, 1.3333)
red_wall = gr.material( {0.6, 0.267, 0.3686}, {0.3, 0.05, 0.05}, 25, 0.0)
green_wall = gr.material( {0.2686, 0.2627, 0.6}, {0.1, 0.05, 0.5}, 25, 0.0)
grey_wall = gr.material( {0.6649, 0.623, 0.7002}, {0.3, 0.3, 0.3}, 25, 0.0)
reflective_wall = gr.material( {0, 0, 0}, {0.9, 0.9, 0.9}, 25, 1e-10)
white_ceiling = gr.material({0.9, 0.9, 0.9}, {0.5, 0.5, 0.5}, 25, 0.0)
mat4  = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25, 0.0)			--disable optics
mat1  = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25, 0.0)  		--disable optics
mat3  = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25, 0.0)			--disable optics
reflective = gr.material({0.0, 0.0, 0.0}, {0.7, 0.7, 0.7}, 25, 0.0)		--disable optics and disfusion
mirror1 = gr.material( {0.2, 0.2, 0.2},   {1.0, 1.0, 1.0},	25, 0.0)
mirror2 = gr.material( {0.2, 0.2, 0.2},   {1.0, 1.0, 1.0},	25, 0.0)
glass2 = gr.material( {0.05, 0.05, 0.05}, {0.7, 0.7, 0.7},  25, 1.5)			--enable optics

marble = gr.material( {0.7, 0.7, 0.7}, {1.0, 1.0, 1.0}, 25, 0.0)
hardwoodfloor = gr.material({0.7, 0.7, 0.7}, {0.1, 0.1, 0.1}, 25, 0.0)

scene = gr.node('scene')

transparentSphere = gr.sphere('transparentSphere')
scene:add_child(transparentSphere)
transparentSphere:set_material(mirror1)
transparentSphere:scale(2, 2, 2)
transparentSphere:translate(0, -3, 4)

transparentCylinder = gr.sphere('transparentCylinder')
transparentSphere:add_child(transparentCylinder)
transparentCylinder:set_material(water)
transparentCylinder:scale(1/4, 1/4, 1/4)
transparentCylinder:translate(3, 3, 3)

transparentCylinder2 = gr.cube('transparentCylinder2')
scene:add_child(transparentCylinder2)
transparentCylinder2:set_material(glass)
transparentCylinder2:scale(1, 2, 1)
transparentCylinder2:translate(3, -7, 4)

floor = gr.mesh('floor', 'plane.obj')
scene:add_child(floor)
floor:set_material(marble)
floor:scale(10, 10, 30)
floor:translate(0, -10, 0)

backwall = gr.mesh('backwall', 'plane.obj')
scene:add_child(backwall)
backwall:set_material(grey_wall)
backwall:rotate('X', 90)
backwall:scale(10, 10, 1)
backwall:translate(0, 0, -10)

leftwall = gr.mesh('leftwall', 'plane.obj')
scene:add_child(leftwall)
leftwall:set_material(reflective_wall)
leftwall:rotate('Z', -90)
leftwall:rotate('X', 90)
leftwall:scale(10, 10, 30)
leftwall:translate(-10, 0, 0)

rightwall = gr.mesh('rightwall', 'plane.obj')
scene:add_child(rightwall)
rightwall:set_material(reflective_wall)
rightwall:rotate('Z', 90)
rightwall:rotate('X', 90)
rightwall:scale(10, 10, 30)
rightwall:translate(10, 0, 0)

topwall = gr.mesh('topwall', 'plane.obj')
scene:add_child(topwall)
topwall:set_material(white_ceiling)
topwall:rotate('X', 180)
topwall:scale(10, 10, 30)
topwall:translate(0, 9.9, 0)

-- The lights
white_light = gr.light({2.0, 2.0, 3}, {0.8, 0.8, 0.8}, {1, 0, 0})
green_light = gr.light({0.0, 0.0, 400.0}, {0.8, 1.0, 0.8}, {1, 0, 0})

gr.render(scene, 'sample.png', 512, 512,
	  {0, 0, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {white_light, green_light})