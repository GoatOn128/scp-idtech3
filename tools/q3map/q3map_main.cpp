int q3map_main( int argc, char **argv );

class idQ3MapApplication {
public:
	int Run( int argc, char **argv ) const {
		return q3map_main( argc, argv );
	}
};

int main( int argc, char **argv ) {
	return idQ3MapApplication().Run( argc, argv );
}
