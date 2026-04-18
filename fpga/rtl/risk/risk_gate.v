// -----------------------------------------------------------------------
// Module: risk_gate
// Description: Wire-speed regulatory compliance engine (SEC 15c3-5)
// Performs Credit Limit, Fat-Finger, and Duplicate detection in < 3.2ns
// -----------------------------------------------------------------------
module risk_gate #(
    parameter PRICE_W = 64,
    parameter QTY_W   = 32,
    parameter LIMIT_NOTIONAL = 64'd1000000000 // $10M in fixed point
) (
    input  wire                 clk,
    input  wire                 rst_n,
    
    // Inbound Order Interface (from RL Core)
    input  wire [PRICE_W-1:0]   in_order_price,
    input  wire [QTY_W-1:0]     in_order_qty,
    input  wire [63:0]          in_order_hash, // Hash for duplicate detection
    input  wire                 in_order_valid,
    
    // Outbound Order Interface (to OUCH Encoder)
    output reg  [PRICE_W-1:0]   out_order_price,
    output reg  [QTY_W-1:0]     out_order_qty,
    output reg                  out_order_valid,
    
    // Risk Violation Alarms
    output reg                  risk_violation_credit,
    output reg                  risk_violation_fat_finger,
    output reg                  risk_violation_duplicate,
    
    // Configurable Thresholds (from Control Plane)
    input  wire [PRICE_W-1:0]   cfg_max_price,
    input  wire [PRICE_W-1:0]   cfg_min_price,
    input  wire [QTY_W-1:0]     cfg_max_qty
);

    // 1. Credit Limit Monitoring (Gross Notional)
    reg [PRICE_W-1:0] total_exposure;
    wire [PRICE_W+QTY_W-1:0] order_notional = in_order_price * in_order_qty;
    
    // 2. Duplicate Detection (Simplified 1-entry Bloom Filter for sub-us window)
    reg [63:0] last_order_hash;
    reg [31:0] timestamp_counter; // For windowing
    
    // Combinatorial Check Logic
    wire check_credit     = (total_exposure + order_notional[PRICE_W-1:0] <= LIMIT_NOTIONAL);
    wire check_fat_finger = (in_order_price <= cfg_max_price) && 
                            (in_order_price >= cfg_min_price) && 
                            (in_order_qty   <= cfg_max_qty);
    wire check_duplicate  = (in_order_hash != last_order_hash) || (timestamp_counter > 32'd322); // 1us window @ 322MHz

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            total_exposure <= 0;
            last_order_hash <= 0;
            timestamp_counter <= 0;
            out_order_valid <= 0;
            risk_violation_credit <= 0;
            risk_violation_fat_finger <= 0;
            risk_violation_duplicate <= 0;
        end else begin
            timestamp_counter <= timestamp_counter + 1;
            
            if (in_order_valid) begin
                if (check_credit && check_fat_finger && check_duplicate) begin
                    // All checks passed: Forward order and update exposure
                    out_order_price <= in_order_price;
                    out_order_qty   <= in_order_qty;
                    out_order_valid <= 1;
                    total_exposure  <= total_exposure + order_notional[PRICE_W-1:0];
                    last_order_hash <= in_order_hash;
                    timestamp_counter <= 0; // Reset duplicate window
                end else begin
                    // Risk violation occurred
                    out_order_valid <= 0;
                    risk_violation_credit     <= !check_credit;
                    risk_violation_fat_finger <= !check_fat_finger;
                    risk_violation_duplicate  <= !check_duplicate;
                end
            end else begin
                out_order_valid <= 0;
            end
        end
    end

endmodule
