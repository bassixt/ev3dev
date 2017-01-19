
void research (donald)

// first scan
for (i = -45 ; i+2 ; i<= 45) {
	
	for (j= 0 ; i++ ; j<5){
		tmp[j] = get_value(IR_sensor);
	}
	
	// calculate the average of measure per each direction to make it more precise
	points[i] = (tmp[0] + tmp[1] + tmp[2] + tmp[3] + tmp[4])/5
	angles[]
}
//looking for the smallest value + correponding direction
	for (i = -45 ; i+2 ; i<= 45i ){
		minimum = min(points[i]);
	}	
diretion = angle[indexOfMinimum];

// move in this direction for 3/4 of the distance
go_ahead(donald, 3/4*minimum, direction);

// second scan like the first scan
[...]
new_minimum
new_direction
// move to grab the ball
go_ahead(3/4*minimum);
grab();