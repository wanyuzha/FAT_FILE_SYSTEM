#pragma once
struct catalog
{
	char filename[8];
	char filename_extension[3];
	char attribute;
	char unuse1;
	char establish_time_precise;
	char establish_time[2];
	char establish_date[2];
	char latest_access_date[2];
	char unuse2[2];
	char modify_time[2];
	char modify_date[2];
	char start_cluster[2];
	char file_size[4];
};