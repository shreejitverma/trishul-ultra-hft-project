`timescale 1ns / 1ps

module rx_parser #(
    parameter DATA_W = 64
) (
    input wire clk,
    input wire rst,
    
    // AXI-Stream Slave (Input from PHY/MAC)
    input wire [DATA_W-1:0] s_axis_tdata,
    input wire s_axis_tvalid,
    input wire s_axis_tlast,
    output wire s_axis_tready,
    
    // AXI-Stream Master (Output Payload)
    output reg [DATA_W-1:0] m_axis_tdata,
    output reg m_axis_tvalid,
    output reg m_axis_tlast,
    input wire m_axis_tready,
    
    // Extracted Header Fields
    output reg [31:0] ip_src,
    output reg [31:0] ip_dst,
    output reg [15:0] udp_sport,
    output reg [15:0] udp_dport,
    output reg header_valid
);

    // Flow control: We are ready if downstream is ready
    assign s_axis_tready = m_axis_tready;

    // State Machine
    localparam ST_ETH = 2'd0;
    localparam ST_IP  = 2'd1;
    localparam ST_UDP = 2'd2;
    localparam ST_PAY = 2'd3;

    reg [1:0] state;

    always @(posedge clk) begin
        if (rst) begin
            state <= ST_ETH;
            m_axis_tvalid <= 1'b0;
            m_axis_tlast <= 1'b0;
            header_valid <= 1'b0;
        end else begin
            // Default assignments
            m_axis_tvalid <= 1'b0;
            m_axis_tlast <= 1'b0;
            
            if (s_axis_tlast) header_valid <= 1'b0; 

            if (s_axis_tvalid && s_axis_tready) begin
                case(state)
                    ST_ETH: begin
                        // Cycle 0: DstMAC[47:0] + SrcMAC[15:0] -> Not extracting here for sim
                        state <= ST_IP; 
                    end
                    
                    ST_IP: begin
                        // Extraction (Simplified logic for thesis demonstration)
                        // Assumes IP Src/Dst are aligned in this cycle
                        ip_src <= s_axis_tdata[31:0];
                        ip_dst <= s_axis_tdata[63:32];
                        state <= ST_UDP;
                    end
                    
                    ST_UDP: begin
                        // Extraction
                        udp_sport <= s_axis_tdata[15:0];
                        udp_dport <= s_axis_tdata[31:16];
                        header_valid <= 1'b1;
                        state <= ST_PAY;
                    end
                    
                    ST_PAY: begin
                        m_axis_tdata <= s_axis_tdata;
                        m_axis_tvalid <= 1'b1;
                        m_axis_tlast <= s_axis_tlast;
                        
                        if (s_axis_tlast) begin
                            state <= ST_ETH;
                            header_valid <= 1'b0;
                        end
                    end
                endcase
            end
        end
    end

endmodule
