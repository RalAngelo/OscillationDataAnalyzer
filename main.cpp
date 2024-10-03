#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// Root-framework
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TStyle.h"

// Class to represent a generic data point with variable columns
class DataPoint {
public:
    float bin_center;
    float ibd_counts;              // Background subtracted IBD counts or Bin Content (Null prediction)
    float total_stats_error;       // Total Stats Error (Prompt data only)
    float background_counts;       // Background Spectrum counts (Prompt data only)
    float background_stats_error;  // Background Stats Error (Prompt data only)

    DataPoint() : bin_center(0), ibd_counts(0), total_stats_error(0),
                  background_counts(0), background_stats_error(0) {}
};

// Structure to hold segment-to-baseline mapping
struct SegmentMap {
    int segment;
    float baseline;
};

// Class to handle data parsing, ROOT file creation, and plotting
class OscillationAnalyzer {
private:
    std::vector<std::vector<DataPoint>> allPromptData;
    std::vector<std::vector<DataPoint>> allNullData;
    std::vector<SegmentMap> segmentMap;
    std::string rootFileNamePrompt;
    std::string rootFileNameNull;

public:
    OscillationAnalyzer(const std::string& rootFileNamePrompt, const std::string& rootFileNameNull)
        : rootFileNamePrompt(rootFileNamePrompt), rootFileNameNull(rootFileNameNull) {}

    // Parse a file based on the number of columns
    std::vector<DataPoint> parseFile(const std::string& filename, int numColumns) {
        std::vector<DataPoint> data;
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return data;
        }

        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);

            DataPoint dp;
            ss >> dp.bin_center;
            ss.ignore(); // Ignore delimiter

            ss >> dp.ibd_counts;

            if (numColumns > 2) {
                ss.ignore();
                ss >> dp.total_stats_error;
                ss.ignore();
                ss >> dp.background_counts;
                ss.ignore();
                ss >> dp.background_stats_error;
            }

            data.push_back(dp);
        }

        infile.close();
        return data;
    }

    // Parse segment map file
    std::vector<SegmentMap> parseSegmentMapFile(const std::string& filename) {
        std::vector<SegmentMap> segmentMap;
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return segmentMap;
        }

        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);

            SegmentMap sm;
            ss >> sm.segment;
            ss.ignore();
            ss >> sm.baseline;

            segmentMap.push_back(sm);
        }

        infile.close();
        return segmentMap;
    }

    // Map segment data to baseline using the segment map
    std::vector<DataPoint> mapSegmentToBaseline(const std::vector<DataPoint>& segmentData, const SegmentMap& segmentMap) {
        std::vector<DataPoint> baselineData = segmentData;
        for (auto& dp : baselineData) {
            dp.bin_center = segmentMap.baseline;  // Replace bin center with the baseline
        }
        return baselineData;
    }

    // Create a ROOT file from the parsed data
    void createRootFile(const std::string& rootFileName, const std::vector<std::vector<DataPoint>>& allData) {
        TFile file(rootFileName.c_str(), "RECREATE");
        TTree tree("OscData", "Oscillation Data");

        float bin_center, ibd_counts, total_stats_error, background_counts, background_stats_error;

        tree.Branch("bin_center", &bin_center, "bin_center/F");
        tree.Branch("ibd_counts", &ibd_counts, "ibd_counts/F");

        // Add branches for prompt data
        tree.Branch("total_stats_error", &total_stats_error, "total_stats_error/F");
        tree.Branch("background_counts", &background_counts, "background_counts/F");
        tree.Branch("background_stats_error", &background_stats_error, "background_stats_error/F");

        for (const auto& segmentData : allData) {
            for (const auto& dp : segmentData) {
                bin_center = dp.bin_center;
                ibd_counts = dp.ibd_counts;
                total_stats_error = dp.total_stats_error;
                background_counts = dp.background_counts;
                background_stats_error = dp.background_stats_error;
                tree.Fill();
            }
        }

        tree.Write();
        file.Close();
    }

    // Plot histograms for null prediction data
    void plotHistograms() {
        TFile file(rootFileNameNull.c_str(), "READ");
        TTree* tree = (TTree*)file.Get("OscData");

        float bin_center;
        float bin_content;
        tree->SetBranchAddress("bin_center", &bin_center);
        tree->SetBranchAddress("ibd_counts", &bin_content);

        TStyle* style = new TStyle("style", "Style for histograms");
        style->SetTitleFontSize(0.05);
        style->SetLabelSize(0.04, "XYZ");
        style->SetTitleSize(0.05, "XYZ");
        style->SetOptStat(0);
        style->cd();

        // Increase canvas size and margins
        TCanvas* c1 = new TCanvas("c1", "Null Oscillation Prediction", 3000, 1200); // Increased canvas width
        c1->SetMargin(0.2, 0.2, 0.2, 0.2); // Increased margins

        c1->Divide(5, 2);

        std::vector<TH1F*> histograms;
        for (int segment = 1; segment <= 10; ++segment) {
            std::string hist_name = "Baseline " + std::to_string(segment);
            TH1F* hist = new TH1F(hist_name.c_str(), hist_name.c_str(), 16, 0.5, 7.5); // Extended x-axis range
            histograms.push_back(hist);
        }

        Long64_t nentries = tree->GetEntries();
        int current_segment = 0;
        for (Long64_t i = 0; i < nentries; ++i) {
            tree->GetEntry(i);
            if (i % 16 == 0 && i != 0) current_segment++;
            histograms[current_segment]->SetBinContent(i % 16 + 1, bin_content);
        }

        for (int i = 0; i < histograms.size(); ++i) {
            c1->cd(i + 1);
            histograms[i]->SetLineColor(kBlack);
            histograms[i]->SetLineWidth(3);
            histograms[i]->SetTitle(("Histogram " + std::to_string(i + 1)).c_str());
            histograms[i]->GetXaxis()->SetTitle("Energy (MeV)");
            histograms[i]->GetYaxis()->SetTitle("Counts");

            // Scale x-axis for 1 MeV = 3 cm
            histograms[i]->GetXaxis()->SetTitleOffset(1.2); // Adjust title offset for visibility
            histograms[i]->GetXaxis()->SetLabelOffset(0.02); // Adjust label offset for visibility

            histograms[i]->GetXaxis()->SetRangeUser(0.5, 7.5); // x-axis range
            histograms[i]->GetXaxis()->SetNdivisions(16); // Number of divisions on x-axis
        
            // Make axis labels bold and larger
            histograms[i]->GetXaxis()->SetLabelFont(62); // Bold font
            histograms[i]->GetYaxis()->SetLabelFont(62); // Bold font
            histograms[i]->GetXaxis()->SetLabelSize(0.05); // Larger size
            histograms[i]->GetYaxis()->SetLabelSize(0.05); // Larger size
        
            histograms[i]->GetXaxis()->SetTitleFont(62); // Bold font
            histograms[i]->GetYaxis()->SetTitleFont(62); // Bold font
            histograms[i]->GetXaxis()->SetTitleSize(0.06); // Larger size
            histograms[i]->GetYaxis()->SetTitleSize(0.06); // Larger size
        
            gPad->SetLeftMargin(0.2); // Margin for Y-axis label visibility
            gPad->SetRightMargin(0.2); // Increased right margin
            gPad->SetTopMargin(0.2); // Increased top margin
            gPad->SetBottomMargin(0.2); // Increased bottom margin

            histograms[i]->Draw("HIST");
        }

        c1->Update();
        c1->SaveAs("Fig40_Reconstructed.png");
    }



    void plotHistograms2() {
        TFile file(rootFileNamePrompt.c_str(), "READ");
        TTree* tree = (TTree*)file.Get("OscData");

        float baseline, ibd_counts;

        tree->SetBranchAddress("bin_center", &baseline);
        tree->SetBranchAddress("ibd_counts", &ibd_counts);

        TH1F* h1 = new TH1F("h1", "IBD Counts vs Baseline; Baseline (m); IBD Counts", 100, 0, 10);

        int nentries = tree->GetEntries();
        std::cout << "The value of nentries is: " << nentries << std::endl;
        for (int i = 0; i < nentries; i++) {
            tree->GetEntry(i);
            h1->Fill(baseline, ibd_counts);
        }

        TCanvas* c2 = new TCanvas("c2", "IBD Counts vs Baseline", 900, 600);

        // Stylizing histogram
        h1->SetLineColor(kBlue);          // Set line color (blue)
        h1->SetLineWidth(3);              // Set line width (thicker)
        h1->SetFillColorAlpha(kCyan-10, 0.3);  // Set fill color with transparency
        h1->SetMarkerStyle(20);           // Set marker style
        h1->SetMarkerSize(1.2);           // Set marker size
        h1->SetMarkerColor(kRed);         // Set marker color

        // Set axis titles
        h1->GetXaxis()->SetTitle("Baseline (m)");
        h1->GetYaxis()->SetTitle("IBD Counts");

        // Set axis title size and font
        h1->GetXaxis()->SetTitleSize(0.05);
        h1->GetXaxis()->SetTitleFont(42);
        h1->GetYaxis()->SetTitleSize(0.05);
        h1->GetYaxis()->SetTitleFont(42);
        h1->GetYaxis()->SetTitleOffset(1.2); // Increase offset for better visibility

        // Draw histogram with styling
        h1->Draw("HIST P E");  // Draw as histogram with markers and error bars

        // Improve canvas aesthetics
        c2->SetGrid();         // Add grid to canvas
        c2->SetFillColor(0);   // Canvas background color
        c2->SetFrameLineWidth(2); // Set frame line width
        c2->SetLeftMargin(0.15); // Increase left margin to make space for y-axis title
        c2->SetBottomMargin(0.15); // Increase bottom margin if needed

        c2->Update();
        c2->SaveAs("Fig40_1_Reconstructed.png");
    }


    void processFiles() {
        // Parse the segment map data
        segmentMap = parseSegmentMapFile("PromptDataSet/1.1_Osc_SegmentMap.txt");

        // Parse prompt data and map to baseline
        for (int segment = 15; segment <= 138; ++segment) {
            std::string filename = "PromptDataSet/1.4_Osc_Prompt" + std::to_string(segment) + ".txt";
            std::vector<DataPoint> data = parseFile(filename, 5); // 5 columns for prompt data

            for (const auto& sm : segmentMap) {
                if (sm.segment == segment) {
                    data = mapSegmentToBaseline(data, sm);
                    break;
                }
            }

            allPromptData.push_back(data);
        }

        // Parse null prediction data
        for (int baseline = 1; baseline <= 10; ++baseline) {
            std::string filename = "NullDataSet/1.6_Osc_NullOscPred" + std::to_string(baseline) + ".txt";
            std::vector<DataPoint> data = parseFile(filename, 2); // 2 columns for null data
            allNullData.push_back(data);
        }

        // Create ROOT files
        createRootFile(rootFileNamePrompt, allPromptData);
        createRootFile(rootFileNameNull, allNullData);

        // Plot histograms for null prediction data
        plotHistograms();
        plotHistograms2();
    }
};

int main() {
    OscillationAnalyzer analyzer("oscPrompt.root", "oscNull.root");
    analyzer.processFiles();
    return 0;
}
