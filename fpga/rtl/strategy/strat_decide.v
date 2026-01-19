`timescale 1ns / 1ps

module strat_decide #(
    parameter W = 32
) (
    input wire clk,
    input wire rst,
    
    // BBO from Order Book
    input wire [W-1:0] bid_px0,
    input wire [W-1:0] ask_px0,
    
    // Strategy Parameters (from Control Plane/AXI)
    input wire [W-1:0] fair_px,
    input wire [W-1:0] thresh_buy,
    input wire [W-1:0] thresh_sell,
    
    // Trigger
    input wire in_valid,
    
    // Signals
    output reg buy,
    output reg sell,
    output reg out_valid
);

    // Combinatorial comparison
    wire buy_cond  = (ask_px0 + thresh_buy < fair_px);
    wire sell_cond = (bid_px0 - thresh_sell > fair_px);

    always @(posedge clk) begin
        if (rst) begin
            buy <= 1'b0;
            sell <= 1'b0;
            out_valid <= 1'b0;
        end else begin
            out_valid <= 1'b0;
            if (in_valid) begin
                // Deterministic single-cycle decision
                buy <= buy_cond;
                sell <= sell_cond;
                out_valid <= 1'b1;
            end
        end
    end

endmodule
