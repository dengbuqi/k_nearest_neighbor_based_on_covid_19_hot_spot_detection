#include"pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <istream>
#include <Windows.h>
#include <tchar.h>
#include <string>
#include <math.h>
// 벡터의 sort와 unique 사용을 위해 적용
#include <algorithm>

using namespace std;

// 엑셀 데이터의 행 
const int row_num = 4001;

// 엑셀 데이터의 열 
const int column_num = 25;

// knn의 k 개수
const int k = 15;

// 행정 지방명 저장
int province_num = 6;

// 지역명 저장
int city_num = 7;

// 엑셀 데이터의 행 크기를 저장하기 위한 변수
int row_idx = 0;

// levenshtein distance를 계산하기 위한 배열
int dist[1001][1001];

// 엑셀 데이터를 읽어오기 위한 벡터
vector <string> csv_read_row(istream &file, char delimiter);

// 행정 지방을 저장하기 위한 벡터
vector<string> province_arr;

// 지역명 저장하기 위한 벡터
vector<string> city_arr;

// 계산된 levenshtein 거리값 저장
vector<int> province_levenshtein_dist;
vector<int> city_levenshtein_dist;

// 엑셀 데이터를 저장하기 위한 2차원 배열
string** infoArray = new string*[row_num];

// 엑셀 데이터를 읽기 위한 파일
std::vector<std::string> csv_read_row(std::istream &file, char delimiter)
{
	stringstream ss;
	bool inquotes = false;
	vector<string> row;

	while (file.good())
	{
		char c = file.get();
		//beginquotechar
		if (!inquotes && c == '"') 
		{
			inquotes = true;
		}
		//quotechar
		else if (inquotes && c == '"') 
		{
			//2 consecutive quotes resolve to 1
			if (file.peek() == '"')
			{
				ss << (char)file.get();
			}
			//endquotechar
			else 
			{
				inquotes = false;
			}
		}
		//end of field
		else if (!inquotes && c == delimiter) 
		{
			row.push_back(ss.str());
			ss.str("");
		}
		else if (!inquotes && (c == '\r' || c == '\n'))
		{
			if (file.peek() == '\n') { file.get(); }
			row.push_back(ss.str());
			return row;
		}
		else
		{
			ss << c;
		}
	}
}

// levenshtein distance 계산
int levenshtein(string& input1, string& input2)
{
	// 비교할 첫 번째 문자열 - 각 행에 삽입
	for (int i = 1; i <= input1.length(); i++)
	{
		dist[i][0] = i;
	}

	// 비교할 두 번째 문자열 - 각 열에 삽입
	for (int j = 1; j <= input2.length(); j++)
	{
		dist[0][j] = j;
	}

	// levenshtein distance 계산
	for (int j = 1; j <= input2.length(); j++)
	{
		for (int i = 1; i <= input1.length(); i++)
		{
			if (input1[i - 1] == input2[j - 1])
			{
				dist[i][j] = dist[i - 1][j - 1];
			}
			else
			{
				dist[i][j] = min(dist[i - 1][j - 1] + 1, 
						     min(dist[i][j - 1] + 1, 
								 dist[i - 1][j] + 1) 
								);
			}
		}
	}

	// 계산된 거리값 출력
	/*for (int j = 0; j <= input2.length(); j++)
	{
		for (int i = 0; i <= input1.length(); i++)
		{
			printf("%d\t", dist[i][j]);
		}
		printf("\n");
	}*/

	// 계산된 거리값 반환
	return dist[input1.length()][input2.length()];
}

// 엑셀 데이터 읽기
void Excel_read(string& path)
{
	ifstream file(path);

	// 엑셀 데이터를 저장하기 위한 문자열 배열 선언
	for (int i = 0; i < row_num; i++)
	{
		infoArray[i] = new string[column_num];
	}

	// 만약 파일이 존재하지 않을 경우의 예외처리
	if (file.fail())
	{
		cout << "파일 없음" << endl;
	}

	// 만약 파일이 존재할 경우 데이터 읽기
	while (file.good())
	{
		// .CSV 파일이므로 ','로 구분
		vector<string> row = csv_read_row(file, ',');

		for (int i = 0; i < column_num; i++)
		{
			// 엑셀데이터에 있는 값 저장
			infoArray[row_idx][i] = row[i];
		}

		// 엑셀데이터에 있는 값 저장
		// 행정지방
		province_arr.push_back(infoArray[row_idx][province_num]);

		//cout << infoArray[row_idx][0]+" | "+ infoArray[row_idx][1] +"|"+infoArray[row_idx][2]+"|"+infoArray[row_idx][3]+"|"+infoArray[row_idx][4]+"|"+infoArray[row_idx][5]+"|"+infoArray[row_idx][6]+"|"+infoArray[row_idx][7]<<endl;
		// 엑셀 데이터의 행 크기만큼 증가
		row_idx++;
	}

	// 파일 종료
	file.close();
}

// 중복된 지역을 제거한 지역 탐색 및 반환
void Vector_processing(vector<string>& vector)
{
	// 벡터 정렬
	sort(vector.begin(), vector.end());

	// 중복 제거
	vector.erase(unique(vector.begin(), vector.end()), vector.end());

	// 중복제거된 벡터 출력
	//cout << "<COVID 19 확진 지역 목록>" << "\n";
	//for (unsigned int i = 0; i < vector.size(); i++)
	//{
	//	cout << vector[i] << "\n";
	//	cout << "-----------" << "\n";
	//}
}

// 최소값에 해당하는 벡터의 인덱스 탐색
int find_min_val(vector<int>& vector, int minimum)
{
	// 최소값 인덱스를 저장할 변수
	int min_val = 0;

	// 계산된 거리 및 지역 출력
	for (int i = 0; i < vector.size(); i++)
	{
		// 최소값 여부 확인
		if (minimum == vector[i])
		{
			min_val = i;
		}
		else
		{
			continue;
		}
	}

	return min_val;
}

// sorting with knn
double* dist_sort(double *dist, int size)
{
	int *index = new int[size];

	for (int i = 0; i < size; i++)
	{
		index[i] = i;
	}

	for (int i = 1; i < size; i++)
	{
		for (int j = 0; j < i; j++)
		{
			if (dist[j] > dist[i])
			{
				int temp = index[i];
				index[i] = index[j];
				index[j] = temp;
				double temp2 = dist[i];
				dist[i] = dist[j];
				dist[j] = temp2;
			}
		}
	}
	return dist;
}

// main문
int main(int argc, char *argv[])
{
	cout << "       COVID-19 risk area detector" << endl;
	cout << "<1. 행정 지방 2. 지역구 3. 성별 4. 생년월일>" << endl;
	cout << "\t" << endl;
	
	// 유저를 통해 입력받는 값
	string province, city, sex;
	int birth;
	int age;

	// 입력
	cin >> province >> city >> sex >> birth;

	// 2020년도를 기준으로 나이 계산
	age = 2020 - birth;

	cout << "\t" << endl;
	cout << "입력하신 값은 행정 지방: " << province << ", 지역구: "<< city << ", 성별: " << sex << ", 생년월일: " << to_string(birth) << "입니다." << endl;
	cout << "위의 주어진 정보를 기반으로 COVID 확진자가 많이 방문한 장소를 비교하겠습니다." << endl;
	cout << "\t" << endl;

	// 행정지방 변수
	string find_province;
	// 지역명 변수
	string find_city;
	//성별에 따른 경로
	string path;
	string female = "female";

	// 엑셀 데이터 파일의 경로
	string path_male   = "C:\\Users\\minseok\\Desktop\\빅데이터 분석\\텀프로젝트\\COVID\\data\\datasets_527325_1205308_PatientInfo_male.csv";
	string path_female = "C:\\Users\\minseok\\Desktop\\빅데이터 분석\\텀프로젝트\\COVID\\data\\datasets_527325_1205308_PatientInfo_female.csv";
	
	//성별 비교
	if (!sex.compare(female))
	{

		path = path_female;
	}
	else
	{
		path = path_male;
	}

	// 엑셀 데이터 읽기
	Excel_read(path);

	// 문자열의 중복 제거를 위한 벡터 프로세싱
	// province: 행정 지방
	// city: 지역명
	Vector_processing(province_arr);
	Vector_processing(city_arr);

	// levenshtein distance 계산 - 행정 지방
	for (int i = 0; i < province_arr.size(); i++)
	{
		// 해당 질의 벡터와 엑셀 데이터 간의 문자열 거리 비교
		province_levenshtein_dist.push_back(levenshtein(province, province_arr[i]));

		// 출력
		/*cout << province << "과 " << province_arr[i] << "간의 levenshtein distance 비교" << endl;
		cout << "--------------------------------------------------------------" << "\n";
		cout << "levenshtein distance: " << levenshtein(province, province_arr[i]) << endl;
		cout << "--------------------------------------------------------------" << "\n";*/
	}

	// 계산된 거리값들 중에서 최소값 반환 - province
	int min_province_val = *min_element(province_levenshtein_dist.begin(), province_levenshtein_dist.end());

	// 최소값에 해당하는 인덱스 탐색 - province
	int min_province_idx = find_min_val(province_levenshtein_dist, min_province_val);

	// 유사도가 가장 높은 행정 지방 선택
	find_province = province_arr[min_province_idx];

	// 같은 행정에 속하는 엑셀 데이터의 인덱스 탐색 및 저장
	vector<int> similar_idx;

	// 엑셀 데이터에서 같은 행정에 속하는 곳 탐색
	for (int i = 0; i < row_num; i++)
	{
		if (infoArray[i][province_num] == find_province)
		{
			similar_idx.push_back(i);
		}
		else
		{
			continue;
		}
	}

	// 유사도가 높은 행정 지방을 탐색한 데이터 - 유사한 지역의 갯수만큼 생성
	string **similar_dat = new string*[similar_idx.size()];

	// 배열 생성
	for (int i = 0; i < similar_idx.size(); ++i)
	{
		similar_dat[i] = new string[column_num];
	}

	// 배열 할당
	for (int i = 0; i < similar_idx.size(); i++)
	{
		for (int j = 0; j < column_num; j++)
		{
			// 같은 행정 지역에 속하는 모든 데이터 값 반환
			similar_dat[i][j] = infoArray[similar_idx[i]][j];
			
			// 같은 행정 지역에 속하는 지역명 반환
			city_arr.push_back(infoArray[similar_idx[i]][city_num]);

			//cout << similar_dat[i][j] << "\t";
			//cout << city_arr[i]<< "\n";
		}
		//cout << "\n";
	}

	// 문자열의 중복 제거를 위한 벡터 프로세싱
	// city: 지역명
	Vector_processing(city_arr);

	// 지역명 중복 제거 확인
	cout << "                   <Levenshtein distance를 통한 유사도 분석 결과>" << endl;
	cout <<"입력하신 "<< province << "과/와 높은 유사도를 갖는 행정 지역은 " << find_province <<"으로 간주되며, " << find_province << "에서 발생한 확진 지역구는 다음과 같습니다." << "\n";
	cout << "\t" << endl;
	for (int i = 0; i < city_arr.size(); i++)
	{
		cout <<"No." << i+1 << "  " << city_arr[i] << "\n";
	}

	// levenshtein distance 계산 - 지역명
	if (!city.empty())
	{
		for (int i = 0; i < city_arr.size(); i++)
		{
			// 해당 질의 벡터와 엑셀 데이터 간의 문자열 거리 비교
			city_levenshtein_dist.push_back(levenshtein(city, city_arr[i]));

			// 출력
			//cout << city << "과 " << city_arr[i] << "간의 levenshtein distance 비교" << endl;
			//cout << "--------------------------------------------------------------" << "\n";
			//cout << "levenshtein distance: " << levenshtein(city, city_arr[i]) << endl;
			//cout << "--------------------------------------------------------------" << "\n";
		}
	}
	else
	{
		cout << "지역을 입력하지 않았습니다." << endl;
	}
	
	// 계산된 거리값들 중에서 최소값 반환 - city
	int min_city_val     = *min_element(city_levenshtein_dist.begin(), city_levenshtein_dist.end());

	// 최소값에 해당하는 인덱스 탐색  - city
	int min_city_idx     = find_min_val(city_levenshtein_dist, min_city_val);

	// 유사도가 가장 높은 지역 선택
	find_city = city_arr[min_city_idx];

	cout << "\t" << endl;
	cout << find_province << "에 속하면서, " << city << "과/와 가장 유사한 지역구는 " << find_city << "로 간주됩니다." << "\n";
	cout << "위의 결과를 기반으로 확진자가 많이 방문한 지역을 kNN을 통해 분석하겠습니다." << endl;

	// 같은 행정에 속하는 엑셀 데이터의 인덱스 탐색 및 저장
	vector<int> total_similar_idx;

	// 엑셀 데이터에서 같은 행정에 속하는 곳 탐색
	for (int i = 0; i < similar_idx.size(); i++)
	{
		if (similar_dat[i][city_num] == find_city)
		{
			total_similar_idx.push_back(i);
		}
		else
		{
			continue;
		}
	}

	// 유사도가 높은 행정 지방과 지역을 탐색한 데이터 저장
	string **total_similar_dat = new string*[total_similar_idx.size()];

	// 생성
	for (int i = 0; i < total_similar_idx.size(); ++i)
	{
		total_similar_dat[i] = new string[column_num];
	}

	// 할당
	for (int i = 0; i < total_similar_idx.size(); i++)
	{
		for (int j = 0; j < column_num; j++)
		{
			// 같은 행정 지역에 속하는 모든 데이터 값 반환
			total_similar_dat[i][j] = similar_dat[total_similar_idx[i]][j];
		}
	}

	// knn distance and sort
	//double **dist = new double*[total_similar_idx.size()];
	double *dist = new double[total_similar_idx.size()];
	// 거리 비교를 위한 행렬
	double **compare_dist = new double*[total_similar_idx.size()];
	for (int i = 0; i < total_similar_idx.size(); ++i)
	{
		//dist[i] = new double[2];
		compare_dist[i] = new double[2];
	}

	// 각 정렬된 거리값에 해당하는 인덱스 반환
	for (int i = 0; i < total_similar_idx.size(); i++)
	{
		// euclidean distance
		dist[i] = sqrt( (pow(stod(total_similar_dat[i][3]) - birth, 2) + pow(stod(total_similar_dat[i][4]) - age, 2)) );

		// 거리값 및 인덱스 저장
		compare_dist[i][0] = dist[i];
		compare_dist[i][1] = i;
	}


	// knn을 위한 거리값 계산 및 정렬
	dist_sort(dist, total_similar_idx.size());

	// 작은 거리값을 갖는 인덱스 탐색
	for (int i = 0; i < total_similar_idx.size(); i++)
	{
		for (int j = 0; j < total_similar_idx.size(); j++)
		{
			if (dist[i] == compare_dist[j][0])
			{
				compare_dist[i][0] = dist[i];
				compare_dist[i][1] = compare_dist[j][1];
			}
			else
			{
				continue;
			}
		}

	}

	// 가장 근접한 거리에 해당하는 k개의 정보 출력
	cout << "\t" << endl;
	cout << "                           <kNN 분석 결과>" << endl;
	cout <<"kNN분석 결과, " << age << "세와 유사한 연령대를 갖는 확진자가 많이 방문한 위험 지역은 다음과 같습니다. " <<"(k= " << k << ")" << endl;
	cout << "(해당 지역내에서 비슷한 연령대의 감염된 확진자가 없을 경우에는 지역내 확진자들 중에서 가장 가까운 연령대로 탐색됩니다.)" << endl;
	cout << "\t" << endl;

	//결과 출력
	int find_idx = 0; 
	// 행정 지역 및 지역구의 학교, 학원 비율
	cout << "높은 유사도로 선택된 행정 지방: " << find_province << "\t" << "지역구: " << find_city << "\n";
	
	find_idx = compare_dist[0][1];

	cout << find_city << "의 "  << "초등학교 수: " << total_similar_dat[find_idx][18] << "\t"
			<< find_city << "의 "  << "유치원 수: " << total_similar_dat[find_idx][19] << "\t"
			<< find_city << "의 "  << "대학교 수: " << total_similar_dat[find_idx][20] << "\t"
			<< find_city << "의 "  << "학원 비율: " << total_similar_dat[find_idx][21] << "\t"
			<< endl;


	// kNN 분석 결과
	cout << "\t" << endl;
	cout << "kNN을 통한 결과" << "\n";
	for (int i = 0; i < k; i++)
	{
		find_idx = compare_dist[i][1];

		cout <<"No."<< i+1 << "\t"
			 << "성별: " << total_similar_dat[find_idx][2] << "\t"
			 << "접촉자수: " << total_similar_dat[find_idx][12] << "\t" 
			 << "확진일자: " << total_similar_dat[find_idx][14] << "\t"
			 << "감염 경로/원인: " << total_similar_dat[find_idx][9] 
			 << endl;
	}

	return 0;
}
