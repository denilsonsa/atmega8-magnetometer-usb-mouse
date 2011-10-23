"use strict";

// Based on:
// http://blogoben.wordpress.com/2011/04/16/webgl-basics-4-wireframe-3d-object/
// http://www.rozengain.com/blog/2010/02/22/beginning-webgl-step-by-step-tutorial/
// http://www.iquilezles.org/apps/shadertoy/
// http://www.iquilezles.org/apps/shadertoy/help.html
// http://cake23.de/diffusion-mix.html
// http://evanw.github.com/webgl-filter/
// http://learningwebgl.com/blog/?p=1786
// http://www.khronos.org/opengles/sdk/docs/reference_cards/OpenGL-ES-2_0-Reference-card.pdf
// http://developer.apple.com/library/ios/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/BestPracticesforShaders/BestPracticesforShaders.html


// Global vars
var canvascontainer, canvas, gl;
var vertex_shader, fragment_shader, shader_program;
var vertex_buffer;
var fb_tex, frame_buffer;
var mouse_x, mouse_y, mouse_click, mouse_hold;

function init_global_vars() {
	canvascontainer = document.getElementById('canvascontainer');
	canvas = document.getElementById('canvas');
	gl = canvas.getContext('experimental-webgl');

	mouse_x = 0.5;
	mouse_y = 0.5;
	mouse_click = false;
	mouse_hold = false;
}

function update_mouse_pos(ev) {
	var rect = this.getBoundingClientRect();
	var x = ev.clientX - rect.left - this.clientLeft + this.scrollLeft;
	var y = ev.clientY - rect.top - this.clientTop + this.scrollTop;

	// Fixing Y orientation for OpenGL
	mouse_x = x;
	mouse_y = canvas.height - y;
}

function update_mouse_down(ev) {
	mouse_click = true;
	mouse_hold = true;
}

function update_mouse_up(ev) {
	mouse_hold = false;
}

function resize_canvas() {
	// https://developer.mozilla.org/en/Determining_the_dimensions_of_elements
	var width = canvascontainer.clientWidth;
	var height = canvascontainer.clientHeight;

	console.log('Resizing to', width, height);

	// http://pl4n3.blogspot.com/2010/02/html5-canvas-with-variable-size.html
	canvas.width = width;
	canvas.height = height;
	gl.viewport(0, 0, width, height);

	//console.log(gl.drawingBufferWidth, gl.drawingBufferHeight)
}

function init_shaders() {
	// Global vars: fragment_shader, shader_program
	var vs_text = document.getElementById('shader-vs').text;
	var fs_text = document.getElementById('shader-fs').text;

	// The vertex shader
	vertex_shader = gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertex_shader, vs_text);
	gl.compileShader(vertex_shader);

	if (!gl.getShaderParameter(vertex_shader, gl.COMPILE_STATUS)) {
		abort_with_message('Error while compiling the vertex shader');
		console.log(vs_text);

		// .deleteShader() is optional here, because the shader object is
		// automatically deleted when WebGLShader object is destroyed.
		// http://www.khronos.org/registry/webgl/specs/1.0/#5.13.9
		//gl.deleteShader(fragment_shader);
		vertex_shader = null;

		return false;
	}

	// The fragment shader
	fragment_shader = gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragment_shader, fs_text);
	gl.compileShader(fragment_shader);

	if (!gl.getShaderParameter(fragment_shader, gl.COMPILE_STATUS)) {
		abort_with_message('Error while compiling the fragment shader');
		console.log(fs_text);

		fragment_shader = null;

		return false;
	}

	// The shader program
	shader_program = gl.createProgram();
	gl.attachShader(shader_program, vertex_shader);
	gl.attachShader(shader_program, fragment_shader);
	gl.linkProgram(shader_program);

	if (!gl.getProgramParameter(shader_program, gl.LINK_STATUS)) {
		abort_with_message('Error while linking the shader program');

		shader_program = null;
		fragment_shader = null;

		return false;
	}

	gl.validateProgram(shader_program);
	if (!gl.getProgramParameter(shader_program, gl.VALIDATE_STATUS)) {
		abort_with_message('Error while validating the shader program');

		shader_program = null;
		fragment_shader = null;

		return false;
	}

	return true;
}
function init_vertex_buffer() {
	var vertices = new Float32Array([
		+1.0, -1.0,  // topright
		+1.0, +1.0,  // bottomright
		-1.0, -1.0,  // topleft
		-1.0, +1.0   // bottomleft
	]);
	vertex_buffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer);
	gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
}
function init_frame_buffer() {
	frame_buffer = gl.createFramebuffer();
	gl.bindFramebuffer(gl.FRAMEBUFFER, frame_buffer);
	frame_buffer.width = 1024;
	frame_buffer.height = 1024;

	fb_tex = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, fb_tex);
	//gl.pixelStorei(gl.GL_UNPACK_ALIGNMENT, 1);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, frame_buffer.width, frame_buffer.height, 0, gl.RGB, gl.UNSIGNED_BYTE, null);

	gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fb_tex, 0);

	// Tidying up GL state
	gl.bindTexture(gl.TEXTURE_2D, null);
    //gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
}

function paint() {
	// Fragment shader
	gl.uniform2f(gl.getUniformLocation(shader_program, "canvassize"), canvas.width, canvas.height);
	gl.uniform2f(gl.getUniformLocation(shader_program, "fbsize"), frame_buffer.width, frame_buffer.height);
	gl.uniform2f(gl.getUniformLocation(shader_program, "mouse"), mouse_x, mouse_y);
	gl.uniform1i(gl.getUniformLocation(shader_program, "mouse_click"), mouse_click);
	gl.uniform1i(gl.getUniformLocation(shader_program, "mouse_hold"), mouse_hold);
	gl.uniform1i(gl.getUniformLocation(shader_program, "fb"), fb_tex);

	mouse_click = false;

	// Using the vertex buffer with the vertex shader
	var pos = gl.getAttribLocation(shader_program, "pos");
	gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer);
	gl.vertexAttribPointer(pos, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(pos);

	// Using the frame buffer
	gl.uniform1i(gl.getUniformLocation(shader_program, "to_fb"), true);
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, fb_tex);
	gl.bindFramebuffer(gl.FRAMEBUFFER, frame_buffer);
	gl.viewport(0, 0, frame_buffer.width, frame_buffer.height);

	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);  // 4 is the number of vertices
	gl.flush();

	gl.uniform1i(gl.getUniformLocation(shader_program, "to_fb"), false);
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, fb_tex);
	gl.bindFramebuffer(gl.FRAMEBUFFER, null);
	gl.viewport(0, 0, canvas.width, canvas.height);

	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);  // 4 is the number of vertices
	gl.flush();

    //gl.disableVertexAttribArray(pos);
}

function animation_frame_callback() {
	paint();
	window.requestAnimationFrame(animation_frame_callback);
}

function abort_with_message(text) {
	var nowebgl = document.getElementById('nowebgl');
	canvascontainer.style.display = 'none';
	nowebgl.style.display = 'block';
	nowebgl.innerHTML = text;
}

function init() {
	init_global_vars();
	if(!gl) {
		abort_with_message('WebGL not supported');
		return;
	}

	resize_canvas();

	window.addEventListener('resize', resize_canvas, false);
	canvas.addEventListener('mousemove', update_mouse_pos, false);
	canvas.addEventListener('mousedown', update_mouse_down, false);
	canvas.addEventListener('mouseup', update_mouse_up, false);

	var stat = init_shaders();
	if (!stat) {
		return;
	}

	init_vertex_buffer();
	init_frame_buffer();

	gl.useProgram(shader_program);

	//gl.clearDepth(1.0);
	//gl.enable(gl.DEPTH_TEST);
	//gl.depthFunc(gl.LEQUAL);

	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT);
	//gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	animation_frame_callback();
}

window.addEventListener('load', init, false);
