`timescale 1ns / 1ps

module order_encode #(
    parameter DATA_W = 64
) (
    input wire clk,
    input wire rst,
    
    // Trigger from Strategy/Risk
    input wire in_valid,
    input wire in_buy,   // 1=Buy, 0=Sell
    input wire [31:0] in_px,
    input wire [31:0] in_qty,
    
    // AXI-Stream Master (Output to MAC TX)
    output reg [DATA_W-1:0] m_axis_tdata,
    output reg m_axis_tvalid,
    output reg m_axis_tlast,
    input wire m_axis_tready
);

    // Simplified OUCH-like message (Fixed 16 bytes = 2 cycles of 64-bit)
    // Cycle 0: [Type(8), Token(32), Side(8), Qty(16)] -> simplified
    // Cycle 1: [Price(32), Stock(32)]
    
    localparam ST_IDLE = 1'b0;
    localparam ST_SEND = 1'b1;
    
    reg state;
    
    always @(posedge clk) begin
        if (rst) begin
            state <= ST_IDLE;
            m_axis_tvalid <= 1'b0;
            m_axis_tlast <= 1'b0;
            m_axis_tdata <= 0;
        end else begin
            m_axis_tvalid <= 1'b0;
            m_axis_tlast <= 1'b0;
            
            case(state)
                ST_IDLE: begin
                    if (in_valid && m_axis_tready) begin
                        // Word 0: [Side(8 bits), Qty(32 bits), Reserved(24 bits)]
                        m_axis_tdata <= { (in_buy ? 8'h42 : 8'h53), in_qty, 24'h0 }; 
                        m_axis_tvalid <= 1'b1;
                        state <= ST_SEND;
                    end
                end
                
                ST_SEND: begin
                    if (m_axis_tready) begin
                        // Word 1: [Price(32 bits), EndTag(32 bits)]
                        m_axis_tdata <= { in_px, 32'hEEEE_EEEE };
                        m_axis_tvalid <= 1'b1;
                        m_axis_tlast <= 1'b1;
                        state <= ST_IDLE;
                    end
                end
            endcase
        end
    end

endmodule
