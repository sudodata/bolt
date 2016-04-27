#include "meshDef.hpp"
#include <fstream>

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
		cout<<"\n(Diagnostic Msg) Writing to file: "<<filename;
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
				size_t facetNo;

				cout<<"\nFile is not ASCII, using binary format";

				file = fopen(filename, "rb");
				
				if(!file) {
					cout<<"\nfile opening failed,  with error ";
					perror("Error");
					return 1;
				}
				
				fread(modelName, 80, 1, file);
				fread((void *)&facetNo, 4, 1, file);
				
				cout<<"\nNo. Of Facets is : "<<facetNo;
				cin.get();


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

void stlMesh::set_min_max_var_z (float &min, float &max) {
	
	for ( auto meshIterator = mesh.begin(); meshIterator != mesh.end(); meshIterator++ ) 
		for ( int i = 0; i < 3; i++ ) {
			
			min_z = ( min_z > meshIterator -> vertex[i].z ) ? meshIterator -> vertex[i].z : min_z;
			max_z = ( max_z < meshIterator -> vertex[i].z ) ? meshIterator -> vertex[i].z : max_z;

	}
	min=min_z;
	max=max_z;
	cout<<"\n(Diagnostic Msg) Minimum and maximum z values are : "<<min_z<<", "<<max_z;
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