
for i=180, 360 do
	glass = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25, 0.3, 1.0)

	color_increase = i / 360
	reflective_increase = 1.0
	red = gr.material({1.0, color_increase, color_increase }, {1.0, color_increase , color_increase }, 25, reflective_increase, 0.0, 1.0)
	blue = gr.material({ color_increase , color_increase, 1.0}, { color_increase, color_increase, 1.0 }, 25, reflective_increase, 0.0, 1.0)
	green = gr.material({ color_increase, 1.0, color_increase }, { color_increase, 1.0, color_increase }, 25, reflective_increase, 0.0, 1.0)

	scene = gr.node( 'scene' )
	scene:translate(0, 0, -800)

	-- Center
	center_x = 0
	center_y = 0
	center_z = -400

	s1 = gr.nh_sphere('s1', {center_x, center_y, center_z}, 150)
	scene:add_child(s1)
	s1:set_material(glass)

	-- Rotation on x axis
	deg = math.rad(3 * i)
	deg_prev = math.rad(3 * i - 10)

	x_rotate_radius = 360
	x_rotate_y = math.cos(deg) * x_rotate_radius
	x_rotate_z = math.sin(deg) * x_rotate_radius

	x_rotate_y_prev = math.cos(deg - deg_prev) * x_rotate_radius
	x_rotate_z_prev = math.sin(deg - deg_prev) * x_rotate_radius

	s2_trail = gr.nh_sphere_mb('s2_trail', {center_x, center_y + x_rotate_y, center_z + x_rotate_z}, 25, { 0, x_rotate_y_prev - x_rotate_y, x_rotate_z_prev - x_rotate_z })
	s2 = gr.nh_sphere('s2', {center_x, center_y + x_rotate_y, center_z + x_rotate_z}, 25)
	scene:add_child(s2_trail)
	scene:add_child(s2)
	s2_trail:set_material(red)
	s2:set_material(red)

	y_rotate_radius = 720 - 2 * i;
	if y_rotate_radius < 360 then
		y_rotate_radius = 360
	end

	y_rotate_x = math.cos(deg) * y_rotate_radius
	y_rotate_z = math.sin(deg) * y_rotate_radius

	y_rotate_x_prev = math.cos(deg - deg_prev) * y_rotate_radius
	y_rotate_z_prev = math.sin(deg - deg_prev) * y_rotate_radius

	s3_trail = gr.nh_sphere_mb('s3_trail', {center_x + y_rotate_x, center_y, center_z + y_rotate_z}, 25, { y_rotate_x_prev - y_rotate_x, 0, y_rotate_z_prev - y_rotate_z})
	s3 = gr.nh_sphere('s3', {center_x + y_rotate_x, center_y, center_z + y_rotate_z}, 25)
	scene:add_child(s3_trail)
	scene:add_child(s3)
	s3_trail:set_material(blue)
	s3:set_material(blue)

	z_rotate_radius = 980 - 3 * i;
	if z_rotate_radius < 360 then
		z_rotate_radius = 360
	end

	z_rotate_x = math.cos(deg) * z_rotate_radius
	z_rotate_y = math.sin(deg) * z_rotate_radius

	z_rotate_x_prev = math.cos(deg - deg_prev) * z_rotate_x
	z_rotate_y_prev = math.sin(deg - deg_prev) * z_rotate_y

	s4_trail = gr.nh_sphere_mb('s4_trail', {center_x + z_rotate_x, center_y + z_rotate_y, center_z}, 25, { z_rotate_x_prev - z_rotate_x, z_rotate_y_prev - z_rotate_y, 0 })
	s4 = gr.nh_sphere('s4', { center_x + z_rotate_x, center_y + z_rotate_y, center_z }, 25)
	scene:add_child(s4_trail)
	scene:add_child(s4)
	s4_trail:set_material(green)
	s4:set_material(green)

	white_light = gr.light({-100.0, 0.0, -400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
	magenta_light = gr.light({400.0, 100.0, -650.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

	eye_z = 200 - i
	if eye_z < -500 then
		eye_z = -500
	end

	gr.render(scene, string.format('animation/final%03d.png', i), 512, 512,
		{ 0, 0,  eye_z}, 
		{ 0, 0, -1 }, {0, 1, 0}, 50,
		{0.3, 0.3, 0.3}, {white_light, magenta_light}, i)
end