


/* The Donald structure */
typedef struct motandsens test;
struct motandsens {
	float x,y;
	float teta;
[...]
};

void positioning (donald){
// to protect the access to sensor
	mutex_lock;
	check_sensor;
	mutex_unlock;
	
// calculation of the rotation
	rotation = - ( new_angle - last_angle);
//refresh last_angle
	last_angle = new_angle;
// get the wheel's positions
	new_left = position_left_wheel(sensor);
	new_right = position_right_wheel(sensor);
// definition of the angle
	new_teta = new_teta + rotation;
// calculation of the displacement on straight line
	displ_left = new_left - old_left;
	displ_right = new_right - old_right;
// average of both sides + conversion into mm with encod_scale
	displ = encod_scale*(displ_left + displ_right)/2;
//update the displacements
	old_left = new_left;
	old_right = new_right;
// calculating the displacements (straigh or turned) = odometry
	old_x = old_x + disp * sign * cos( new_teta );
	old_y = old_y + disp * sign * sin( new_teta ); 
//update the Donald structure 
	donald->teta=new_teta;
 	donald->x = old_x;
 	donald->y = old_y;
	
}

