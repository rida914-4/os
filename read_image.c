#include<stdio.h>



	/* Our structure */
	struct rec
	{
		int x,y,z;
	};

	int main()
	{
		int counter;
		FILE *ptr_myfile;
		struct rec my_record;
		
		FILE *ptr_outputfile;			// write
		struct rec write_record;			// write

		ptr_myfile=fopen("/home/rida/Pictures/new.jpg","rb");
		
		ptr_outputfile=fopen("test.jpg","wb"); 	// write 
		
		
		if (!ptr_myfile)
		{
			printf("Unable to open file!");
			return 1;
		}
		for ( counter=1; counter <= 10; counter++)
		{
			fread(&my_record,sizeof(struct rec),1,ptr_myfile);
			printf("%d\n",my_record.x);
			fwrite(&write_record, sizeof(struct rec), 1, ptr_outputfile);
			
		}
		fclose(ptr_myfile);
		fclose(ptr_outputfile);
		return 0;
	}
