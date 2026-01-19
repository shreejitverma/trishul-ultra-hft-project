`timescale 1ns / 1ps

/**
 * strat_decide.v
 * 
 * Implements Fig 5 (RL Core) from Thesis.
 * Architecture:
 *   1. Feature Extraction (Registers)
 *   2. Weight ROM
 *   3. DSP48 MAC Core
 *   4. Activation (ReLU)
 *   5. Comparator
 */
module strat_decide #(
    parameter W = 32,
    parameter FEAT_DIM = 4, // Number of features
    parameter FIXED_POINT = 8 // Decimal bits
) (
    input wire clk,
    input wire rst,
    
    // BBO from Order Book
    input wire [W-1:0] bid_px0,
    input wire [W-1:0] ask_px0,
    
    // Strategy Parameters (System State)
    input wire [W-1:0] fair_px,
    input wire signed [W-1:0] inventory,
    input wire [W-1:0] volatility,
    
    // Trigger
    input wire in_valid,
    
    // Output
    output reg buy,
    output reg sell,
    output reg out_valid
);

    // --- 1. Feature Extraction (Stage 1) ---
    // Features:
    // 0: Spread (ask - bid)
    // 1: Book Imbalance (simulated here as 0 for simplicity or fair - mid)
    // 2: Inventory
    // 3: Volatility
    
    reg signed [W-1:0] feat_vec [0:FEAT_DIM-1];
    reg valid_s1;
    
    always @(posedge clk) begin
        if (rst) begin
            valid_s1 <= 0;
            feat_vec[0] <= 0; feat_vec[1] <= 0; feat_vec[2] <= 0; feat_vec[3] <= 0;
        end else if (in_valid) begin
            feat_vec[0] <= $signed(ask_px0 - bid_px0); // Spread
            feat_vec[1] <= $signed(fair_px - ((ask_px0 + bid_px0) >> 1)); // Alpha
            feat_vec[2] <= inventory;
            feat_vec[3] <= $signed(volatility);
            valid_s1 <= 1;
        end else begin
            valid_s1 <= 0;
        end
    end

    // --- 2. Weight ROM (Constant for now) ---
    // In real FPGA, this is a BRAM or Distributed RAM
    // Weights (Fixed Point 8.8):
    // w0 = -1.5 (Avoid high spread)
    // w1 =  2.0 (Follow Alpha)
    // w2 = -0.5 (Mean reversion on inventory)
    // w3 = -0.1 (Avoid high vol)
    
    wire signed [15:0] weights [0:FEAT_DIM-1];
    assign weights[0] = -16'sd384; // -1.5 * 256
    assign weights[1] =  16'sd512; //  2.0 * 256
    assign weights[2] = -16'sd128; // -0.5 * 256
    assign weights[3] = -16'sd25;  // -0.1 * 256

    // --- 3. DSP MAC (Stage 2) ---
    // sum = f0*w0 + f1*w1 + ...
    reg signed [47:0] dot_prod; // Wider accumulator
    reg valid_s2;

    integer i;
    always @(posedge clk) begin
        if (rst) begin
            dot_prod <= 0;
            valid_s2 <= 0;
        end else if (valid_s1) begin
            // In synthesis, this unrolls to DSP cascade or adder tree
            dot_prod <= (feat_vec[0] * weights[0]) + 
                        (feat_vec[1] * weights[1]) + 
                        (feat_vec[2] * weights[2]) + 
                        (feat_vec[3] * weights[3]);
            valid_s2 <= 1;
        end else begin
            valid_s2 <= 0;
        end
    end

    // --- 4. Activation (Stage 3) ---
    // ReLU: max(0, x)
    reg signed [47:0] activated_val;
    reg valid_s3;

    always @(posedge clk) begin
        if (rst) begin
            activated_val <= 0;
            valid_s3 <= 0;
        end else if (valid_s2) begin
            if (dot_prod > 0)
                activated_val <= dot_prod;
            else
                activated_val <= 0;
            valid_s3 <= 1;
        end else begin
            valid_s3 <= 0;
        end
    end

    // --- 5. Comparator / Decision (Stage 4) ---
    // Threshold check
    localparam THRESHOLD = 48'sd25600; // 100.0 * 256 (arbitrary)

    always @(posedge clk) begin
        if (rst) begin
            buy <= 0;
            sell <= 0;
            out_valid <= 0;
        end else if (valid_s3) begin
            out_valid <= 1;
            // Simple logic: if score high, buy; if inventory high (negative weight handled), sell?
            // Simplified: High score = Buy. Very Low score (handled by separate path in real life) = Sell.
            // Here we just map positive high score to Buy.
            
            if (activated_val > THRESHOLD) begin
                buy <= 1;
                sell <= 0;
            end else begin
                // In this simple demo, we don't sell. 
                // Real logic would have two heads (Buy Score, Sell Score).
                buy <= 0;
                sell <= 0;
            end
        end else begin
            out_valid <= 0;
            buy <= 0;
            sell <= 0;
        end
    end

endmodule