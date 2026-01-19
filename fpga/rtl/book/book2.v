`timescale 1ns / 1ps

module book2 (
    input wire clk,
    input wire rst,
    
    // Input from Decoder
    input wire tick_valid,
    input wire tick_type, // 0 = Add, 1 = Exec
    input wire tick_side, // 1 = Buy, 0 = Sell
    input wire [31:0] tick_qty,
    input wire [31:0] tick_price,
    
    // L1 Output (Best Bid / Best Offer)
    output reg [31:0] bid_px0,
    output reg [31:0] bid_sz0,
    output reg [31:0] ask_px0,
    output reg [31:0] ask_sz0
);

    localparam INVALID_PX_ASK = 32'hFFFFFFFF;
    localparam INVALID_PX_BID = 32'h00000000;

    always @(posedge clk) begin
        if (rst) begin
            bid_px0 <= INVALID_PX_BID;
            bid_sz0 <= 0;
            ask_px0 <= INVALID_PX_ASK;
            ask_sz0 <= 0;
        end else begin
            if (tick_valid) begin
                if (tick_type == 1'b0) begin // ADD ORDER
                    if (tick_side == 1'b1) begin // BUY
                        if (tick_price > bid_px0 || bid_px0 == INVALID_PX_BID) begin
                            bid_px0 <= tick_price;
                            bid_sz0 <= tick_qty;
                        end else if (tick_price == bid_px0) begin
                            bid_sz0 <= bid_sz0 + tick_qty;
                        end
                    end else begin // SELL
                        if (tick_price < ask_px0 || ask_px0 == INVALID_PX_ASK) begin
                            ask_px0 <= tick_price;
                            ask_sz0 <= tick_qty;
                        end else if (tick_price == ask_px0) begin
                            ask_sz0 <= ask_sz0 + tick_qty;
                        end
                    end
                end
                // Simplified: Not handling Executions/Cancels in this L1 baseline snippet
                // as it requires a full order-ID map memory (TCAM/RAM) which is Phase 4.
            end
        end
    end

endmodule
