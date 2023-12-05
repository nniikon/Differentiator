include config.mk

all:
	@$(MAKE) -C ./common/logs
	@$(MAKE) -C ./common/memAllocations
	@$(MAKE) -C ./binaryTree/source
	@$(MAKE) -C ./source
	@$(MAKE) -C ./treeParser
	@$(MAKE) -C ./common/fileToBuffer
	@$(MAKE) -C ./common/stack
	@$(GXX) $(CFLAGS) -o $(BUILD_DIR)/$(EXEC_NAME) $(BUILD_DIR)/*.o

clean:
	@rm -rf $(BUILD_DIR)

run: 
	$(BUILD_DIR)/$(EXEC_NAME)