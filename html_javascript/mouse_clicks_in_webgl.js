"use strict";

// Based on:
// http://blogoben.wordpress.com/2011/04/16/webgl-basics-4-wireframe-3d-object/
// http://www.rozengain.com/blog/2010/02/22/beginning-webgl-step-by-step-tutorial/
// http://www.iquilezles.org/apps/shadertoy/
// http://cake23.de/diffusion-mix.html
// http://evanw.github.com/webgl-filter/


// Global vars
var canvascontainer, canvas, gl;
var vertex_shader, fragment_shader, shader_program;
var vertex_buffer;

function init_global_vars() {
	canvascontainer = document.getElementById('canvascontainer');
	canvas = document.getElementById('canvas');
	gl = canvas.getContext('experimental-webgl');
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

function paint() {
	var pos = gl.getAttribLocation(shader_program, "pos");
	gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer);
	gl.vertexAttribPointer(pos, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(pos);
	// 4 is the number of vertices
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	gl.flush();
    gl.disableVertexAttribArray(pos);
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

	var stat = init_shaders();
	if (!stat) {
		return;
	}

	init_vertex_buffer();

	gl.useProgram(shader_program);

	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);

	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	setTimeout(paint, 1000);
}

window.addEventListener('load', init, false);
