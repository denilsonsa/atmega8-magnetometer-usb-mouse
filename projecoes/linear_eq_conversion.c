#include <stdio.h>
#include <string.h>
#include <math.h>

// Compatibility begin  {{{

#define FIX_POINTER(x)
#define uchar  unsigned char
#define int    short int

// Compatibility end  }}}

// Definitions copied from sensor.h begin  {{{
typedef struct XYZVector {
	int x, y, z;
} XYZVector;

typedef struct SensorEepromData {
	// This struct is used for data at EEPROM and at SRAM

	// Boolean to enable Zero compensation
	uchar zero_compensation;

	// Zero calibration value
	XYZVector zero;

	XYZVector corners[4];
} SensorEepromData;

typedef struct SensorData {
	union {
		uchar flags;
		struct {
			// Boolean that detects if the sensor have reported an overflow
			uchar overflow:1;

			// Set to 1 whenever new sensor data has been read and hasn't
			// been used yet. This flag should be cleared elsewhere, after
			// using the data.
			uchar new_data_available:1;

			// Almost the same as TWI_statusReg.lastTransOK.
			// Gets set whenever a function returns with SENSOR_FUNC_ERROR.
			// Gets reset whenever a function returns with SENSOR_FUNC_DONE.
			uchar error_while_reading:1;

			// Enable continuous reading of sensor values
			// This variable should be used in main() main loop (together
			// with a timer) to detect when sensor_read_data_registers()
			// should be called.
			uchar continuous_reading:1;

			uchar unused_bits:4;
		};
	};

	// The X,Y,Z data from the sensor
	XYZVector data;

	SensorEepromData e;

	// Zero calibration temporary values
	XYZVector zero_min;
	XYZVector zero_max;

	// Used to determine the next step in non-blocking functions.
	// Must be set to zero to ensure each function starts from the beginning.
	uchar func_step;

} SensorData;

// Definitions copied from sensor.h end  }}}

// Definitions copied from mouseemu.h begin  {{{
typedef struct MouseReport {
	uchar report_id;
	int x; // 0..32767
	int y; // 0..32767
	uchar buttons;
} MouseReport;
// Definitions copied from mouseemu.h end  }}}

SensorData sensor;
MouseReport mouse_report;



static void fill_matrix_from_sensor(float m[3][4]) {  // {{{
	SensorData *sens = &sensor;
	FIX_POINTER(sens);

	// The linear system:
	// -t*P + u*(B-A) + v*(C-A) = -A
	// Where:
	//   A = topleft
	//   B = topright
	//   C = bottomleft
	//   P = current point
	// The final, (x,y) screen coordinates are (u,v)
	//
	// That system is equivalent to this one:
	// A + u*(B-A) + v*(C-A) = t*P
	// Which means the current point is equal to the topleft corner, plus "u"
	// times in the topleft/topright direction, plus "v" times in the
	// topleft/bottomleft direction. "u" and "v" are between 0.0 and 1.0.

	// First column: - current_point
	m[0][2] = -sens->data.x;
	m[1][2] = -sens->data.y;
	m[2][2] = -sens->data.z;

	// Note: the values below are constant, and could have been pre-converted
	// to float only once (either at boot, or after updating those values).
	// It would save some cycles during this runtime.

	// Second column: topright - topleft
	m[0][0] = sens->e.corners[1].x - sens->e.corners[0].x;
	m[1][0] = sens->e.corners[1].y - sens->e.corners[0].y;
	m[2][0] = sens->e.corners[1].z - sens->e.corners[0].z;

	// Third column: bottomleft - topleft
	m[0][1] = sens->e.corners[2].x - sens->e.corners[0].x;
	m[1][1] = sens->e.corners[2].y - sens->e.corners[0].y;
	m[2][1] = sens->e.corners[2].z - sens->e.corners[0].z;

	// Fourth column: - topleft
	m[0][3] = -sens->e.corners[0].x;
	m[1][3] = -sens->e.corners[0].y;
	m[2][3] = -sens->e.corners[0].z;
}  // }}}

static void swap_rows(float a[4], float b[4]) {  // {{{
	float tmp;
	uchar i;

	for (i=0; i<4; i++) {
		tmp = a[i];
		a[i] = b[i];
		b[i] = tmp;
	}
}  // }}}

static uchar mouse_axes_linear_equation_system() {  // {{{
#define W 4
#define H 3
	// Matrix of 3 lines and 4 columns
	float m[H][W];
	// The solution of this system
	float sol[H];

	uchar y;

	fill_matrix_from_sensor(m);

	// Gauss-Jordan elimination, based on:
	// http://elonen.iki.fi/code/misc-notes/python-gaussj/index.html

	for(y = 0; y < H; y++) {
		uchar maxrow;
		uchar y2;
		float pivot;

		// Finding the row with the maximum value at this column
		maxrow = y;
		pivot = fabs(m[maxrow][y]);
		for(y2 = y+1; y2 < H; y2++) {
			float newpivot;
			newpivot = fabs(m[y2][y]);
			if (newpivot > pivot) {
				pivot = newpivot;
				maxrow = y2;
			}
		}
		// If the maximum value in this column is zero
		if (pivot < 0.0009765625) {  // 2**-10 == 0.0009765625
			// Singular
			return 0;
		}
		if (maxrow != y) {
			swap_rows(m[y], m[maxrow]);
		}

		// Now we are ready to eliminate the column y, using m[y][y]
		for(y2 = y+1; y2 < H; y2++) {
			uchar x;
			float c;
			c = m[y2][y] / m[y][y];

			// Subtracting from every element in row y2
			for(x = y; x < W; x++) {
				m[y2][x] -= m[y][x] * c;
			}
		}
	}

	// The matrix is now in "row echelon form" (it is a triangular matrix).
	// Instead of using a loop for back-substituting all 3 elements from
	// sol[], I'm calculating only 2 of them, as only those are needed.
	sol[2] = m[2][3] / m[2][2];

	sol[1] = m[1][3] / m[1][1] - m[1][2] * sol[2];

	mouse_report.x = (int)(sol[1] * 32767);
	mouse_report.y = (int)(sol[2] * 32767);
	// sol[0] is discarded

	return 1;
#undef W
#undef H
}  // }}}

#undef int



int main(int argc, char *argv[]) {

	short int x, y, z;
	XYZVector* next_vector;

	// By default, store numbers at the sensor data.
	next_vector = &sensor.data;

	while (1) {
		if (scanf("%hd%hd%hd", &x, &y, &z) == 3) {
			// Save it to the SensorData struct
			next_vector->x = x;
			next_vector->y = y;
			next_vector->z = z;

			if (next_vector == &sensor.data) {
				// Do the conversion
				mouse_axes_linear_equation_system();
				printf("%d %d\n", mouse_report.x, mouse_report.y);
			}

			// Next one gets stored at the sensor data.
			next_vector = &sensor.data;
		} else {
			char s[64];
			if (scanf(" %63s", s) == 1) {
				if (strcmp(s, "topleft") ==  0) {
					next_vector = &sensor.e.corners[0];
				} else if (strcmp(s, "topright") ==  0) {
					next_vector = &sensor.e.corners[1];
				} else if (strcmp(s, "bottomleft") ==  0) {
					next_vector = &sensor.e.corners[2];
				} else if (strcmp(s, "bottomright") ==  0) {
					next_vector = &sensor.e.corners[3];
				}
			} else {
				puts("scanf failed. This shouldn't happen. Aborting...");
				return 1;
			}
		}
	}

	return 0;
}

// vim:noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker foldmarker={{{,}}}
