# Oscillation Data Analyzer

This project provides a C++ program that reads oscillation data, processes it, and generates histograms using the ROOT framework. It deals with data from two sources: prompt data and null prediction data. The program parses these files, processes them to map segment data to baseline values, and creates ROOT files and histograms for analysis.

## Features

- Parses oscillation data from text files with configurable columns.
- Maps segment data to baseline values using a segment map file.
- Creates ROOT files with oscillation data.
- Plots histograms of oscillation data using the ROOT framework.

## Dependencies

To run this project, you need the following installed on your system:

- [ROOT](https://root.cern/) - A framework for data processing and visualization.
- A C++ compiler, such as g++.

## Code Structure

- `DataPoint` class: Represents a data point with fields like bin center, IBD counts, total stats error, background counts, and background stats error.
- `SegmentMap` struct: Holds the mapping between segment and baseline values.
- `OscillationAnalyzer` class: Handles file parsing, ROOT file creation, and histogram plotting.

### Files:

- **1.1_Osc_SegmentMap.txt**: Contains mapping from segment to baseline.
- **1.4_Osc_Prompt[segment].txt**: Contains prompt data for different segments.
- **1.6_Osc_NullOscPred[baseline].txt**: Contains null oscillation predictions for different baselines.

## How to Use

1. Clone the repository:
    ```bash
    git clone https://github.com/your_username/oscillation-analyzer.git
    cd oscillation-analyzer
    ```

2. Make sure you have ROOT installed. You can check by running:
    ```bash
    root --version
    ```

3. Compile the code:
    ```bash
    g++ -o oscAnalyzer oscAnalyzer.cpp `root-config --cflags --glibs`
    ```

4. Prepare the data files. Make sure you have the necessary datasets:
   - **1.1_Osc_SegmentMap.txt** 
   - **1.4_Osc_Prompt[segment].txt**
   - **1.6_Osc_NullOscPred[baseline].txt**

5. Run the executable:
    ```bash
    ./oscAnalyzer
    ```

6. The program will process the files, generate ROOT files (`oscPrompt.root` and `oscNull.root`), and produce histograms saved as PNG images:
   - `Fig40_Reconstructed.png`
   - `Fig40_1_Reconstructed.png`

## Code Explanation

The code consists of the following steps:
1. **File Parsing**: Data is parsed from text files into `DataPoint` objects.
2. **Mapping**: Segment data is mapped to baseline values using a segment map.
3. **ROOT File Creation**: The parsed data is stored in ROOT files for further analysis.
4. **Plotting**: Histograms of the data are plotted and saved as images.

## Customization

- Modify the segment and baseline range by adjusting the loops in the `processFiles` method.
- Change the appearance of the histograms by modifying the `plotHistograms` and `plotHistograms2` methods.
- If your data format changes, adjust the `parseFile` method to handle different column structures.

## Contribution

Feel free to fork the project, create issues, or submit pull requests if you have ideas for improvements or bug fixes.

## License

This project is licensed under the MIT License.

