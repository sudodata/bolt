#include "meshDef.hpp"
#include <fstream>
#include <time.h>

float plane::distanceFromPoint (vec3 point) {
	normal = glm::normalize (normal);
	return dot(point, normal) - distance;
}	

void slice::display_slice () {
		cout<<"\n(Diagnostic Msg)\n";

		for (auto sliceIterator = slice.begin(); sliceIterator != slice.end(); sliceIterator++ ) {
	
			cout<<"\nLineSegment: "<<sliceIterator->startpoint<<"\t"<<sliceIterator->endpoint;
		}
		cout<<"\n\n";
	}

void slice::store_slice(string &filename, const int sliceNo) {

		filename = filename + to_string(sliceNo) + ".dat";

		ofstream file;
		file.open (filename);
			
		for ( auto sliceIterator = slice.begin(); sliceIterator != slice.end(); sliceIterator++ ) 			
				file<<sliceIterator->startpoint<<" "<<sliceIterator->endpoint<<"\n";
			
		file.close();
}

void stlMesh::display_all_elements () {

	int counter=0;

	cout<<"\n(Diagnostic Msg)Data in Mesh : ";
	for( auto meshIterator=mesh.begin();meshIterator!=mesh.end();meshIterator++ )				
		cout<<"\nTriangle No. "<<++counter<<" "<<*meshIterator;

}

int stlMesh::readStlFile ( const char *filename ) {
			
	bool isASCII=false; //false -binary, true -ASCII

		triangle t;
		vec3 vertex[4];

		//detect if the file is ASCII or not
		int parser;
		
		FILE *file = fopen(filename, "r");
			//Using C style file IO over C++ style streams, as streams cause data corruption.
			//May be fixable, need to check advantages of both approaches
			
		do{
			parser=fgetc(file);
		}while((parser!=EOF) && parser<=127 );
			//Check if file is ASCII. If true, every character read from file should be <=127
			//Need to check for robustness of this approach.
			
		fclose(file);

		if(parser==EOF)
			isASCII=true;

		if(!isASCII) {
			//block to read binary stl file
			
				char modelName[80], discarder[2];
				long long facetNo;

				cout<<"\nFile is not ASCII, using binary format";

				file = fopen(filename, "rb");
				
				if(!file) {
					cout<<"\nfile opening failed,  with error ";
					perror("Error");
					return 1;
				}
				
				fread(modelName, 80, 1, file);
				fread((void *)&facetNo, 4, 1, file);
				
				cout<<"\nNo. Of Triangles is : "<<facetNo;

				while(!feof(file)){
					for(int i=0;i<4;i++){

						fread((void *)&vertex[i].x, 4, 1 , file);
						fread((void *)&vertex[i].y, 4, 1 , file);
						fread((void *)&vertex[i].z, 4, 1 , file);
				}
						
				vertex>>t;

					mesh.push_back(t);

					fread((void *)discarder, 2, 1, file);
						//discard 2 bytes end of each loop
				}
			return 0;
		}

		else{
		//block for ASCII
		
		cout<<"\nUsing ASCII format";
		
		file = fopen(filename, "r");
		
		char header[6], modelName[256];
		fscanf(file,"%s",header);
			//Again, choosing C style formatted read of C++ style stream.
			//Check advantages of each approach, change if beneficial.
			
		if(!strcmp(header,"solid")){

			fgets(modelName,255,file);
			cout<<"\nModel Name is : "<<modelName;

			while (!feof(file)){

				char string0[11], string1[11];
				
				fscanf(file,"%s", string0);

				if(!strcmp(string0,"endsolid")) {

					cout<<"\nEnd of file reached";
					break;
				}

				fscanf(file,"%s", string1);

				if( !strcmp(string0, "facet") && !strcmp(string1,"normal")) {

					fscanf(file, "%f", &vertex[0].x);
					fscanf(file, "%f", &vertex[0].y);
					fscanf(file, "%f", &vertex[0].z);

					fscanf(file, "%s", string0);
					fscanf(file, "%s", string1);
						
					if( !strcmp(string0, "outer") && !strcmp(string1,"loop")){
					

						for(int i=1; i<=3; i++){

								fscanf(file, "%s", string0);
								
								if( !strcmp(string0, "vertex")){

									fscanf(file, "%f", &vertex[i].x);
									fscanf(file, "%f", &vertex[i].y);
									fscanf(file, "%f", &vertex[i].z);			

								}			
						}
						fscanf(file, "%s", string0);
						if( strcmp(string0, "endloop"))
						{
							cout<<"\nERROR - Expected 'endloop', found "<<string0<<" instead";
							return 1;
						}

						fscanf(file, "%s", string0);

						if( strcmp(string0, "endfacet")) {

							cout<<"ERROR - expected 'endfacet', found "<<string1<<" instead";
							return 1;
						}
					}
				}

				vertex>>t;
				mesh.push_back(t);

			}
		}
	
		else
			cout<<"\nHeader Mismatch, incorrect ASCII file";

		return 0;
	}
		return 1;
}

void stlMesh::set_min_max_var_z (float &minz, float &maxz, float &minx, float &maxx, float &miny, float &maxy) {
	
	for ( auto meshIterator = mesh.begin(); meshIterator != mesh.end(); meshIterator++ ) 
		for ( int i = 0; i < 3; i++ ) {
			
			min_z = ( min_z > meshIterator -> vertex[i].z ) ? meshIterator -> vertex[i].z : min_z;
			max_z = ( max_z < meshIterator -> vertex[i].z ) ? meshIterator -> vertex[i].z : max_z;
			min_x = ( min_x > meshIterator -> vertex[i].x ) ? meshIterator -> vertex[i].x : min_x;
			max_x = ( max_x < meshIterator -> vertex[i].x ) ? meshIterator -> vertex[i].x : max_x;
			min_y = ( min_y > meshIterator -> vertex[i].y ) ? meshIterator -> vertex[i].y : min_y;
			max_y = ( max_y < meshIterator -> vertex[i].y ) ? meshIterator -> vertex[i].y : max_y;

	}

	minz=min_z;
	maxz=max_z;
	minx=min_x;
	maxx=max_x;
	miny=min_y;
	maxy=max_y;
	
}

void stlMesh::recenter(const float xshift, const float yshift, const float zshift ) {
		for ( auto meshIterator = mesh.begin(); meshIterator != mesh.end(); meshIterator++) {
			for( int i =0; i <3; i++ ) {
				meshIterator->vertex[i].x -= xshift;
				meshIterator->vertex[i].y -= yshift;
				meshIterator->vertex[i].z -= zshift;
			}
		}
}

void stlMesh::slice_mesh ( plane *p, slice *s) {

			for(auto meshIterator = mesh.begin(); meshIterator != mesh.end(); meshIterator++) {

				vector<vec3> intersections;

				for (int i=0; i < 3; i++) {

				float dp1 = p->distanceFromPoint(meshIterator->vertex[i]);
				float dp2 = p->distanceFromPoint(meshIterator->vertex[(i+1)%3]);
					
				if( dp1*dp2 < 0) 
					intersections.push_back (
							meshIterator->vertex[i] + 
							((meshIterator->vertex[(i+1)%3] - meshIterator->vertex[i]) * 
							 (dp1/(dp1-dp2))) );
				
				}	

				if(intersections.size()==2) {
					  s->slice.push_back( linesegment(intersections[1], intersections[0]));		
				}
			}
}

void stlMesh::sliceByTriangle(plane *pstart, slice *sstart, float sliceSize, int arr_len){

	plane *p;
	slice *s;

	int sliceCounter;

	float triangle_min_z, triangle_max_z;

	// iterate through all triangles
	for(auto t = mesh.begin(); t!=mesh.end(); t++){
		
		p=pstart;
		s=sstart;
		sliceCounter=0;
		
		triangle_min_z=t->vertex[0].z;
		triangle_max_z=t->vertex[0].z;
	
		// store the bottom-most and top-most z-coordinates of the triangle
		for (int j=1; j<3; j++)	{

			if(t->vertex[j].z<triangle_min_z) triangle_min_z=t->vertex[j].z;
			if(t->vertex[j].z>triangle_max_z) triangle_max_z=t->vertex[j].z;
		}

		// move to the bottom-most plane that cuts the triangle
		while(p->distance<=triangle_min_z && sliceCounter<arr_len){

				p++;
				s++;
				sliceCounter++;

 		}

		// possibly overshot the min-z value, so move back one plane
		if(p!=pstart && sliceCounter>0){
			p--;
			s--;
			sliceCounter--;

		}

		// move through planes till the plane is above the triangle
		while(p->distance<=triangle_max_z && sliceCounter<arr_len && sliceCounter >=0){

//			cout<<"\nSlice No "<<sliceCounter;
		
			vector<vec3> intersections;

//			cout<<"\n Slice "<<sliceCounter;
			for (int i=0; i < 3; i++) {

				float dp1 = p->distanceFromPoint(t->vertex[i]);
				float dp2 = p->distanceFromPoint(t->vertex[(i+1)%3]);


			//	cout<<"\nVertex 1 x: "<<t->vertex[i].x<<" Vertex 1 y: "<<t->vertex[i].y
			//		<<	" Vertex 2 x: "<<t->vertex[(i+1)%3].x<<" Vertex 2 y: "<<t->vertex[(i+1)%3].y
			//		<<	"\nPlane Distnace: "<<p->distance<<" d1: "<<dp1<<" d2: "<<dp2;

				if( dp1*dp2 < 0){ 
					intersections.push_back (t->vertex[i] + ((t->vertex[(i+1)%3] - t->vertex[i]) * 
							 (dp1/(dp1-dp2))) );
				}	

			}
			
			if(intersections.size()==2) 
					 s->slice.push_back( linesegment(intersections[1], intersections[0]));		
				
			p++;
			s++;
			sliceCounter++;
			
 		}
	}
}
